
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

std::vector<double> computeRRCCoeff(int sps, double rolloff,
                                    double domain_range) {
  double t = -1 * (domain_range);
  std::vector<double> taps(2 * domain_range * sps);
  std::cout << "Taps count = " << taps.size() << std::endl;

  int i = 0; // working tap
  double tap = 0;
  while (t <= domain_range) {
    std::cout << "T = " << t << ", tap=" << i << "\n";
    if (t == 0) {
      tap = (1 - rolloff) + (4 * rolloff / M_PI);
    } else {
      if ((t == 1 / (4 * rolloff)) || (t == -1 / 4 * rolloff)) {
        tap = rolloff / std::sqrt(2) *
                  ((1 + 2 / M_PI) * std::sin(M_PI / (4 * rolloff))) +
              (1 - 2 / M_PI) * std::cos(M_PI / (4 * rolloff));
            } else {
              tap = ((std::sin(M_PI * t * (1 - rolloff)) + 4) *
                     (rolloff * t * std::cos(M_PI * t * (1 + rolloff))) /
                     (M_PI * t * std::pow(1 - (4 * rolloff * t), 2)));
            }
    }
    t = t + (1 / (double)sps);
    taps[i] = tap;
    i++; // next tap
  }
  return taps;
}

std::vector<std::complex<double>> computeCRRCCoeff(int sps, double rolloff,
                                                   double domain_range) {
  std::vector<double> taps = computeRRCCoeff(sps, rolloff, domain_range);
  std::vector<std::complex<double>> ctaps;
  ctaps.resize(taps.size());
  for (int i = 0; i < taps.size(); ++i) {
    ctaps[i] = (taps[i], taps[i]);
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

