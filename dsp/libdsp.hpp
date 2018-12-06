#pragma once
#include <complex>
#include <vector>
#include <string>
#include <cstdint>
#include <queue>
#include <cmath>
#include <algorithm>

/////////////////////////////
// Type definitions
///////////////////////////

// Samples per Second
using SampleRate   =    double;
// Frequency in Hz
using FreqHz       =    double;
// Frequency in Rads
using FreqRads     =    double;
// Normalized Freq (Fraction of samplerate)
using NormFreq     =    double;
// Radian Rate (Rads/Sample)
using RadRate      =    double;
// Phase (rads -pi to pi)
using Phase        =    double;
// Amplitude/Magnitude of a signal
using Magnitude    =    double;

// Complex Samples
// define what a complex sample is
using CSample       =    std::complex<double>;
// define what a vector of csamples is
using CSampleVector =    std::vector<CSample>;
// CSample Vector Iterator
using CSampleVectorIter = CSampleVector::iterator;
// Define what a csample Queue is
using CSampleQueue =     std::queue<CSample>;

// Read Samples
// define what a complex sample is
using Sample       =     double;
// define what a vector of samples is
using SampleVector =    std::vector<Sample>;
// Sample Vector Iterator
using SampleVectorIter = SampleVector::iterator;
// Define what a sample Queue is
using SampleQueue =     std::queue<Sample>;

// Compute normalized frquency from a frequency in Hz
NormFreq computeNormFreqRads(SampleRate s, FreqRads f);
// Compute normalized frquency from a frequency in Rads/sec
NormFreq computeNormFreqHz(SampleRate s, FreqHz f);
// Compute RadRate (rads/samp) for a signal in Hz
RadRate computeRadRateFromFreqHz( SampleRate s, FreqHz f );
// Compute RadRate (rads/samp) for a signal in Rads/sec
RadRate computeRadRateFromFreqRads( SampleRate s, FreqRads f );
// Compute wrapped phase (wrap phase at limits to keep it in -pi to +pi
Phase wrapphase(Phase p);
// get Magnatude of a CSample
Magnitude getMagnitude( CSample s );
// get the phase of a CSample (rads)
Phase getPhase( CSample s );
// convert a mag/phase back into a CSample
CSample Polar2CSample( Magnitude m, Phase p );

// NCO object (Cos wave)
struct NCO {
    // radian rate phase acc increases by every sample
    RadRate rate;
    // phase acc
    Phase phase_acc;
    // quick constructor
    NCO( RadRate _r, Phase _p ) : rate(_r), phase_acc(_p)  {}
    // complex next sample, add offset to phase_acc
    Sample generate( Phase offset );
};

// Complex NCO object (COS)
struct CNCO {
    // radian rate phase acc inc by every sample
    RadRate rate;
    // phase accumulator
    Phase phase_acc;
    // simple constructor
    CNCO( RadRate _r, Phase _p ) : rate(_r), phase_acc(_p) {}
    // generate next sample, add offset to phase_acc
    CSample generate( Phase offset );
};

// FIR Filter for real values
struct FIRFilter {
    std::vector<double> coeff;
    std::vector<Sample> taps;
    FIRFilter( std::vector<double> &_coeff );
    Sample process(Sample in);
};

// FIR Filter for complex values
struct CFIRFilter {
    std::vector< std::complex<double> > coeff;
    std::vector<CSample> taps;
    CFIRFilter( std::vector< std::complex<double> > &_coeff );
    CSample process(CSample in);
};

// compute the coeffs needed for a FIR filter
// with sps Samples/Symbol (>2) and with rolloff (0-1)
// domain range give the number of sync cycles to produce for (2,4,6 typically)
// Larger domain ranges will give larger filters with better approximiations.
// Larger number of Samples/Symbol will also cause the filter to become large.
std::vector<double> computeRRCCoeff(int sps, double rolloff,
                                    double domain_range = 4);

// sample thing, returns matching I/Q coeff pairs.
std::vector<std::complex<double>> computeCRRCCoeff(int sps, double rolloff,
                                                   double domain_range = 4);

// apply a window to a set of double values
void applyWindowHann(std::vector<double> v);
