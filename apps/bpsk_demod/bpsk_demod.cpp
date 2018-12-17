#include <iostream>
#include <string>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "libdsp.hpp"

using namespace std;

void printHelp() {
    std::cout << "BPSK Demodulator Application\n\n";
    std::cout << "This application reads a input of samples (c64) and tries to\n";
    std::cout << "perform carrier wipeoff of a BPSK signal input.\n";
    std::cout << "The output contains samples that have been carrier resolved.\n\n";
    std::cout << "Program Options:\n";
    std::cout << "   -i -- (required) File of input complex double samples.\n";
    std::cout << "   -o -- (required) File of output complex double samples. \n";
    std::cout << "   -h -- help message\n";
    std::cout << std::endl;
}

off_t tell (int filedes ) {
    return lseek(filedes, 0L, SEEK_CUR);
}

// returns 0 if parse completes, -1 if parse is incomplete.
int getOptions( int argc, char**argv, std::string &input_file, std::string &output_file ) {
    // get input file of samples to process
    int c;
    while (( c = getopt( argc, argv, "i:o:h") ) != -1  ) {
        switch (c) {
            case 'h':
                printHelp();
                return -1;
                break;
            case 'i':
                input_file = optarg;
                break;
            case 'o':
                output_file = optarg;
                break;
            default:
                std::cout << "Unknown input option provided, try -h for options list.." << std::endl;
                return -1;
        }
    }

    if ( input_file.length() == 0 ) {
        std::cout << "Must specify input sample source (-i)\n";
        return -1;
    }
    if ( output_file.length() == 0 ) {
        std::cout << "Must specify output sample dest (-o)\n";
        return -1;
    }
    return 0;
}


void printDemodStatus(double progress, BpskDemod &demod) {
    // assume VT100 compatible terminal (linux/bsd/etc..)
    //std::cout << "\x1b[2J"; // clear screen
    std::cout << "Demodulator Status:\n";
    std::cout << "Percentage through file: " << progress*100 << "%" << std::endl;
    std::cout << "Demodulator State      : ";
    switch(demod.state) {
        case 0:
            std::cout << "freq acq" << std::endl;
            break;
        case 1:
            std::cout << "phase acq" << std::endl;
            break;
        case 2:
            std::cout << "Tracking" << std::endl;
            break;
        default:
            std::cout << "Unknown state (" << demod.state << ")" << std::endl;
    }
    std::cout << "phase_est              : " << demod.phase_est << std::endl;
    std::cout << "freq_est               : " << demod.freq_est << std::endl;
    std::cout << std::endl << std::flush;
}

int main( int argc, char **argv ) {
    std::string input_file("");
    std::string output_file("");
    int fhi, fho; // file handles
    int bytes_in;

    if ( getOptions(argc, argv, input_file, output_file) < 0 ) {
        std::cout << "Exit..\n" << std::endl;
        return -1;
    }

    // open input file
    fhi = open( input_file.c_str(), O_RDONLY, 0666 );
    if ( fhi < 1 ) {
        std::cout << "Failed to open input file : " << input_file << std::endl;
        return -1;
    }

    // open output file
    fho = open( output_file.c_str(), O_WRONLY | O_CREAT, 0666 );
    if ( fho < 1 ) {
        std::cout << "Failed to open output file : " << output_file << std::endl;
        return -1;
    }

    std::cout << "Input/Output files have been openned succesfully\n";
    std::cout << "Starting BPSK Carrier wipeoff..\n";

    bytes_in = 1;

    BpskDemod demod(4,0.35,256);
    CSample input;
    CSample output;

    // get length of input file
    off_t input_len = lseek(fhi, 0, SEEK_END);
    lseek(fhi,0,SEEK_SET); // seek back to start of file.
    off_t read_pos = 0;
    double progress = 0.0;
    int samp_cntr = 0;

    // read 1 sample from file per loop iteration
    // bytes_in = 0 when end of file is reached.
    while ( (bytes_in = read(fhi, &input, sizeof(CSample) )) >= sizeof(CSample) ) {

        read_pos = tell(fhi);

        // Process Sample
        output = demod.process( input );

        // write output sample
        write( fho, &output, sizeof(CSample) );

        // status print
        if ( demod.PhaseErrorAcc->current_win_value == demod.PhaseErrorAcc->window_size ) {
            if ( samp_cntr == 10 ) {
                // compute current progress
                progress = ((double)read_pos)/((double)input_len);
                printDemodStatus(progress, demod);
                samp_cntr = 0;
            } else {
                samp_cntr++;
            }
        }

    }

    std::cout << "End of Run Status:\n";
    progress = ((double)read_pos)/((double)input_len);
    printDemodStatus(progress, demod);
    std::cout << "Normal Exit..\n";
    return 0;
}


