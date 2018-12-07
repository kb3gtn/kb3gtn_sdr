#include "libdsp.hpp"
#include <complex>
#include <cstdlib>
#include <ctime>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <unistd.h>

using namespace std;

double randval() {
  return ((((double)std::rand() / (double)RAND_MAX) * 2) - 1);
}

int main() {
  std::srand(std::time(nullptr));
  cout << "starting..\n";
  vector<complex<double>> coeff = computeCpxRRC(4, 0.35, 4);
  cout << "coeff count: " << coeff.size() << endl;
  for (auto &c : coeff)
    cout << c << ", ";
  cout << std::endl;
  CFIRFilter rrc_filter(coeff);
  cout << "Setup input/output vectors\n";
  std::vector< complex<double> > *inputvec = new std::vector<complex<double>>(8192);
  std::vector< complex<double> > *outputvec = new std::vector<complex<double>>(8192);
  cout << "Running random stuff though filter..\n";
  for (int i = 0; i < inputvec->size(); ++i) {
    (*inputvec)[i] = std::complex<double>(randval(), randval());
    (*outputvec)[i] = rrc_filter.process( (*inputvec)[i] );
  }
  // print first couple of samples..
  std::cout << "first 64 of samples..\n";
  for (int i = 0; i < 64; ++i) {
    std::cout << i << " -- input: " << (*inputvec)[i]
              << "  output: " << (*outputvec)[i] << std::endl;
  }

  int fh = open("samples.c64", O_CREAT | O_TRUNC | O_WRONLY, 0666);
  if (fh > 1) {
    write(fh, outputvec->data(), outputvec->size() * sizeof((*outputvec)[0]));
  }
  close(fh);
  std::cout << "Written sample sim to samples.c64\n";
  return 0;
}

