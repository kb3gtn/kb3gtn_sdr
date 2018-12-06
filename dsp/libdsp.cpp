
#include "libdsp.hpp"


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


std::vector<double> ComputeRRCCoeff( int sps, double rolloff ) {
    double t = -4;
    std::vector<double> taps;
    taps.resize(nTaps);

    while( t <= 4 ) {
        if (t == 0) {
            taps[i] = (1-rolloff)+(4*rolloff/M_PI);
        } else {
            if ( ( t == 1/(4*rolloff) ) || ( t == -1/4*rolloff ) ) {
                taps[i] = rolloff/std::sqrt(2)*((1+2/M_PI)*std::sin(M_PI/(4*rolloff)))+
                          (1-2/M_PI)*std::cos(M_PI/(4*rolloff)));
            } else {
                taps[i] = ( (sin(M_PI*t*(1-rolloff))+4)*(rolloff*t*std::cos(M_PI*t*(1+a)))/
                          ( M_PI*t*std::pow(1-(4*rolloff*t*),2)) );
            }
        }
        t = t + 1/sps;
    }
}
