
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

FIRFilter::FIRFilter( std::vector<double> &_coeff ) {
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

CFIRFilter::CFIRFilter( std::vector< std::complex<double> > &_coeff ) {
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

void applyCpxWindowHann(std::vector<std::complex<double>> v) {
  double el = v.size();
  for (int i = 0; i < el; ++i) {
    double scalar = 0.5 * (1 - (std::cos((2 * M_PI * i) / (el - 1))));
    v[i] = v[i] * scalar;
  }
}

