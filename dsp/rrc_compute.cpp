#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;
std::vector<double> computeRRC(double sps, double a, double d);


int main() {
    cout << "computing RRC response for:\n";
    double sps = 4;
    cout << "    samples/symbol => " << sps << "\n";
    double a = 0.35;
    cout << "    rolloff (a) => " << a << "\n";
    double d = 4;
    cout << "    duration (d) => " << -1.0*d << "-> +" << d << "\n";

    std::vector<double> coeff = computeRRC( sps, a, d );
    std::cout << "Number of Taps computed: " << coeff.size() << "\n";
    std::cout << "Coefficents calculated:\n";
    for (auto &c: coeff)
        std::cout << c << ", ";
    std::cout << "\n";
    return 0;
}

std::vector<double> computeRRC(double sps, double a, double d) {
    double tap_count = ( sps*2.0*d )+1.0;
    std::vector<double> p(tap_count);
    std::fill(p.begin(), p.end(), 0.0);
    std::vector<double>::iterator cp = p.begin(); // current tap iterator.
    for ( double t=-1*d; t <= d+(1.0/sps); t=t+(1.0/sps) ) {
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

