
#include "libdsp.hpp"
#include <iostream>

NormFreq computeNormFreqRads(SampleRate s, FreqRads f) {
    return s/(f/2*M_PI);
}

NormFreq computeNormFreqHz(SampleRate s, FreqHz f) {
    return s/f;
}

RadRate computeRadRateFromFreqHz( SampleRate s, FreqHz f ) {
    return (2*M_PI*f)/s;
}

RadRate computeRadRateFromFreqRads( SampleRate s, FreqRads f ) {
    return s/f;
}

Phase wrapPhase(Phase p) {
    Phase result = p;
    while (result >= M_PI )
        result = result - 2*M_PI;

    while (result < -M_PI )
        result = result + 2*M_PI;

    return result;
}

Sample NCO::generate( Phase offset ) {
    // compute output sample for current state
    Sample s;
    s = std::cos(phase_acc);
    // update for next sample
    phase_acc = wrapPhase( phase_acc + rate + offset );
    // return computed sample
    return s;
}

CSample CNCO::generate( Phase offset ) {
    CSample s;
    s.real( std::cos(phase_acc) );
    s.imag( std::sin(phase_acc) );
    // update for next sample
    phase_acc = wrapPhase( phase_acc + rate + offset );
    // return computed sample
    return s;
}

Magnitude getMagnitude( CSample s ) {
    return abs(s);
}

Phase getPhase( CSample s ) {
    return arg(s);
}

CSample Polar2CSample( Magnitude m, Phase p ) {
    return std::polar(m,p);
}

FIRFilter::FIRFilter( std::vector<double> _coeff ) {
    // grab local copy of coefficents
    coeff = _coeff;
    // need 1 tap for coeff
    taps.resize( coeff.size() );
    // zero out taps
    for ( auto& t : taps ) { t = 0; }
}

Sample FIRFilter::process(Sample in) {
    // shift in new sample
    std::rotate( taps.begin(), taps.begin()+taps.size()-1, taps.end() );
    taps[0] = in;
    // compute output sample given current state
    Sample out = 0;
    // scale taps and sum..
    for ( int idx=0; idx < taps.size(); ++idx ) {
        out = out + ( coeff[idx] * taps[idx] );
    }
    // return result
    return out;
}

CFIRFilter::CFIRFilter( std::vector< std::complex<double> > _coeff ) {
    // grab local copy of coefficents
    coeff = _coeff;
    // need 1 tap for coeff
    taps.resize( coeff.size() );
    // zero out taps
    for ( auto &t : taps ) { t = (0,0); }
}

CSample CFIRFilter::process(CSample in) {
    // shift in new sample
    std::rotate( taps.begin(), taps.begin()+taps.size()-1, taps.end() );
    taps[0] = in;
    // compute output sample given current state
    CSample out = (0,0);
    // scale taps and sum..
    for ( int idx=0; idx < taps.size(); ++idx ) {
        out = out + ( coeff[idx] * taps[idx] );
    }
    // return result
    return out;
}

std::vector<double> computeRRC(double sps, double a, double d) {
    double tap_count = ( sps*2.0*d )+1.0;
    std::vector<double> p(tap_count);
    std::fill(p.begin(), p.end(), 0.0);
    std::vector<double>::iterator cp = p.begin(); // current tap iterator.
    for ( double t=-1*d; t < d+(1.0/sps); t=t+(1.0/sps) ) {
        std::cout << "T=" << t << std::endl;
        if ( t == 0 ) {
            std::cout << "(debug) t=0 case hit..\n";
            *cp = (1-a)+4.0*a/M_PI; // OK
        } else {
            if ( ( t == 1.0/(4.0*a)) || (t == -1.0/(4.0*a)) ) {
                std::cout << "(debug) t=1/(4*a) case hit..\n";
                *cp = a/std::sqrt(2)*((1.0+2.0/M_PI)*std::sin(M_PI/(4.0*a))+(1.0-2.0/M_PI)*std::cos(M_PI/(4.0*a)));
            } else {
                // matlab: (sin(pi*-4*(1-a))+4*a*-4*cos(pi*-4*(1+a)))/(pi*-4*(1-(4*a*-4)^2))
                *cp =(sin(M_PI*t*(1-a))+4*a*t*cos(M_PI*t*(1+a)))/(M_PI*t*(1-pow(4*a*t,2)));
            }
        }
        cp++; // next tap.
    }
    // Normalize unit energy
    // get squared sum
    double ss=0;
    for ( auto &t: p)
        ss = ss + (t*t);
    // compute the normalization value (nv)
    double nv = std::sqrt(ss);
    // apply normalization value to all taps
    for ( auto &t: p )
        t = t / nv;
    // return taps computed
    return p;
}

std::vector<std::complex<double>> computeCpxRRC(double sps, double a,
                                                   double d) {
  std::vector<double> taps = computeRRC(sps, a, d);
  std::vector<std::complex<double>> ctaps;
  ctaps.resize(taps.size());
  for (int i = 0; i < taps.size(); ++i) {
    ctaps[i] = std::complex<double>(taps[i], taps[i]);
  }
  return ctaps;
}

// apply a hann window to a set of double values
void applyWindowHann(std::vector<double> v) {
  double el = v.size();
  for (int i = 0; i < el; ++i) {
    double scalar = 0.5 * (1 - (std::cos((2 * M_PI * i) / (el - 1))));
    v[i] = v[i] * scalar;
  }
}

// apply a hann window to I/Q values.
void applyCpxWindowHann(std::vector<std::complex<double>> v) {
  double el = v.size();
  for (int i = 0; i < el; ++i) {
    double scalar = 0.5 * (1 - (std::cos((2 * M_PI * i) / (el - 1))));
    v[i] = v[i] * scalar;
  }
}

// Accumulate and Dump  (complex and normal)
CAccumulateAndDump::CAccumulateAndDump( int _window_size, CSample init_val) {
    window_size = _window_size;
    current_win_value = 0;
    accumulator = 0;
    lastDumpValue = init_val;
}

CSample CAccumulateAndDump::process( CSample input ) {
    if ( current_win_value == window_size ) {
        lastDumpValue = accumulator;
        accumulator = CSample(0,0);
        current_win_value = 0;
    }
    accumulator += input;
    current_win_value++;
    return lastDumpValue;
}

AccumulateAndDump::AccumulateAndDump( int _window_size, Sample init_val) {
    window_size = _window_size;
    current_win_value = 0;
    accumulator = 0;
    lastDumpValue = init_val;
}

Sample AccumulateAndDump::process( Sample input ) {
    if ( current_win_value == window_size ) {
        lastDumpValue = accumulator;
        accumulator = 0;
        current_win_value = 0;
    }
    accumulator += input;
    current_win_value++;
    return lastDumpValue;
}

Phase PhaseDetectorBPSK( CSample input ) {
    Phase error_out;
    Phase abs_phase = getPhase(input);
    // compute error from BPSK Reference Constelation points 0 and +/-PI
    if ( input.real() >= 0 ) {
        error_out = abs_phase;
    }
    if ( input.real() < 0 ) {
        if ( input.imag() > 0 ) {
            error_out = abs_phase - M_PI;
        } else {
            error_out = (-1)*(abs_phase + M_PI);
        }
    }
    return error_out;
}


SampleDelay::SampleDelay( int delay_cnt ) {
    delay_reg.resize(delay_cnt);
    std::fill(delay_reg.begin(), delay_reg.end(), 0 );
    read_idx = 1;
    write_idx = 0;
}

Sample SampleDelay::process(Sample input ) {
    delay_reg[write_idx] = input;
    Sample output = delay_reg[read_idx];
    write_idx++;
    read_idx++;
    if ( write_idx == delay_reg.size() ) {
        write_idx = 0;
    }
    if ( read_idx == delay_reg.size() ) {
        read_idx = 0;
    }
    return output;
}

CSampleDelay::CSampleDelay( int delay_cnt ) {
    delay_reg.resize(delay_cnt);
    std::fill(delay_reg.begin(), delay_reg.end(), 0 );
    read_idx = 1;
    write_idx = 0;
}

CSample CSampleDelay::process(CSample input ) {
    delay_reg[write_idx] = input;
    CSample output = delay_reg[read_idx];
    write_idx++;
    read_idx++;
    if ( write_idx == delay_reg.size() ) {
        write_idx = 0;
    }
    if ( read_idx == delay_reg.size() ) {
        read_idx = 0;
    }
    return output;
}


BpskDemod::BpskDemod( int sps, double alpha, int winsize ) {
    Filter = std::make_shared<CFIRFilter>( computeCpxRRC(sps, alpha, 4 ) );
    FreqErrorAcc = std::make_shared<AccumulateAndDump>(winsize);
    PhaseErrorAcc = std::make_shared<AccumulateAndDump>(winsize);
    PhaseDelay = std::make_shared<SampleDelay>(1);
    NCO = std::make_shared<CNCO>(0,0);
    phase_est = 0;
    freq_est = 0;
    state = acq_freq;
}

CSample BpskDemod::process( CSample input) {
    // forward part of loop
    NCO->rate = freq_est;
    CSample wb_sample = NCO->generate(phase_est) * input;
    CSample nb_sample = Filter->process(wb_sample);
    // feedback loop
    Phase phase_err = PhaseDetectorBPSK(nb_sample);
    Phase phase_err_d1 = PhaseDelay->process(phase_err);
    Phase delta_phase_err = phase_err - phase_err_d1;
    // accumlate and scale output to give average
    double AvgFreqError = FreqErrorAcc->process(delta_phase_err) / FreqErrorAcc->window_size;
    double AvgPhaseError = PhaseErrorAcc->process(phase_err) / PhaseErrorAcc->window_size;
    // state machine, update estimate for next sample input.
    switch (state) {
        case acq_freq:
            phase_est = 0;
            freq_est = AvgFreqError;
            if ( AvgFreqError < freq_lock_threshold ) {
                state == acq_phase;
            }
            break;
        case acq_phase:
            phase_est = AvgPhaseError;
            if ( AvgPhaseError < phase_lock_threshold ) {
                state == track;
            }
            if ( AvgFreqError > freq_lock_threshold ) {
                state == acq_freq;
            }
            break;
        case track:
            phase_est = 0.25*AvgPhaseError;
            freq_est = 0.1*AvgFreqError;
            if ( AvgPhaseError > phase_lock_threshold) {
                state == acq_phase;
            }
            if ( AvgFreqError > freq_lock_threshold ) {
                state == acq_freq;
            }
    }

    return nb_sample;
}


