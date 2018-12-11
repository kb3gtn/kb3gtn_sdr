#include <iostream>
#include <string>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

// program reads samples from file <input> of type <T1>
// converts each sample to type T2 and write to file <output>

// CLI Args:
// -s <input filename> -i <Input Type> -d <output filename> -o <output type>
// Types can be "float" and "double"

enum type_t {
    type_none,
    type_float,
    type_double
};

std::string toString(type_t t) {
    switch (t) {
        case type_none:
            return std::string("none");
        case type_float:
            return std::string("float");
        case type_double:
            return std::string("double");
        default:
            return std::string("unknown");
    }
}

// prototypes
int convert_float_to_double( std::string input_file, std::string output_file );
int convert_double_to_float( std::string input_file, std::string output_file );


int main( int argc, char **argv) {
    type_t input_type=type_none;
    type_t output_type=type_none;
    std::string input_file("");
    std::string output_file("");
    int c;

    // parse CLI
    while (( c = getopt(argc,argv, "s:i:d:o:h") ) != -1 ) {
        switch(c) {
            case 'h':
                std::cout << "Sample converter app:" << std::endl;
                std::cout << "Convert a file of complex floats (.c32) to a file of complex doubles (.c64) and vice versa." << std::endl;
                std::cout << "Usage: " << std::endl;
                std::cout << " " << argv[0] << " -s <src file> -i <input type> -d <dest_file> -o <output type>" << std::endl;
                std::cout << " Types can be: <float> or <double> " << std::endl;
                std::cout << std::endl;
                return 0;  // exit normally. do not continue program.
            case 's':
                // input filename
                input_file = optarg;
                break;
            case 'i':
                if ( strcmp("float", optarg) == 0 ) {
                    input_type = type_float;
                } else {
                    if ( strcmp("double", optarg ) == 0 ) {
                        input_type = type_double;
                    } else {
                        std::cout << "Unknown type for input type: " << optarg << std::endl;
                        return -1;
                    }
                }
                break;
            case 'o':
                if ( strcmp("float", optarg) == 0 ) {
                    output_type = type_float;
                } else {
                    if ( strcmp("double", optarg ) == 0 ) {
                        output_type = type_double;
                    } else {
                        std::cout << "Unknown type for input type: " << optarg << std::endl;
                        return -1;
                    }
                }
                break;
            case 'd':
                output_file = optarg;
                break;
            default:
                std::cout << "Unknown Parameters provided..  try -h" << std::endl;
                return -1; // exit app on error
        }
    }

    // check we have all required arguments:
    if ( input_file.length() == 0 ) {
        std::cout << "Must specify src file.. (-s)" << std::endl;
        return -1;
    }
    if ( output_file.length() == 0 ) {
        std::cout << "Must specify dest file.. (-d)" << std::endl;
        return -1;
    }
    if ( input_type == type_none ) {
        std::cout << "Must specify input file type.. (-i) " << std::endl;
        return -1;
    }
    if ( output_type == type_none ) {
        std::cout << "Must specify output file type.. (-o)" << std::endl;
        return -1;
    }
    if ( input_type == output_type ) {
        std::cout << "Why convert from " << toString(input_type) << " to same type " << toString(output_type) << "? (EXIT..)\n ";
        return -2;
    }

    std::cout << "Processing Data given the following parameters:" << std::endl;
    std::cout << "Input File " << input_file << " of type " << toString(input_type) << std::endl;
    std::cout << "Output File " << output_file << " of type " << toString(output_type) << std::endl;

    // determine direction of conversion:
    if ( ( input_type == type_float ) && ( output_type == type_double ) ) {
        return convert_float_to_double( input_file, output_file );
    } else {
        return convert_double_to_float( input_file, output_file );
    }
}


int convert_float_to_double( std::string input_file, std::string output_file ) {
    int fh = open( input_file.c_str(), O_RDONLY, 0666 );
    if ( fh < 1 ) {
        std::cout << "Failed to open input file.. (EXIT)" << std::endl;
        return -1;
    }
    int fho = open ( output_file.c_str(), O_WRONLY | O_CREAT, 0666 );
    if ( fho < 1 ) {
        std::cout << "Failed to open output file.. (EXIT)" << std::endl;
    }
    float inputbuffer[256];
    double outputbuffer[256];
    int bytes;
    int float_cnt;
    int loopctr = 0;

    while (1) {
      // read file into buffer
      bytes = read( fh, inputbuffer, 256*sizeof(float) );
      if ( bytes == 0 ) {
          // end of file reached..
          break; // exit while loop.
      }
      if ( bytes % (sizeof(float)) != 0 ) {
          std::cout << "Warning: Stream error occured, non multiple of 4 bytes read, data truncated." << std::endl;
      }
      float_cnt = bytes / sizeof(float);
      for ( int i=0; i < float_cnt; ++i ) {
          outputbuffer[i] = (double)inputbuffer[i];
      }
      write(fho, outputbuffer, float_cnt*sizeof(double) );
      if ( loopctr == 1024 ) {
          std::cout << "." << std::flush; // progress indicator
          loopctr = 0;
      } else {
        loopctr++;
      }
    }
    std::cout << "\nConversion of file complete." << std::endl;
    return 0;
}


int convert_double_to_float( std::string input_file, std::string output_file ) {
    int fh = open( input_file.c_str(), O_RDONLY, 0666 );
    if ( fh < 1 ) {
        std::cout << "Failed to open input file.. (EXIT)" << std::endl;
        return -1;
    }
    int fho = open ( output_file.c_str(), O_WRONLY | O_CREAT, 0666 );
    if ( fho < 1 ) {
        std::cout << "Failed to open output file.. (EXIT)" << std::endl;
    }
    double inputbuffer[256];
    float outputbuffer[256];
    int bytes;
    int double_cnt;
    int loopctr = 0;

    while (1) {
      // read file into buffer
      bytes = read( fh, inputbuffer, 256*sizeof(double) );
      if ( bytes == 0 ) {
          // end of file reached..
          break; // exit while loop.
      }
      if ( bytes % (sizeof(double)) != 0 ) {
          std::cout << "Warning: Stream error occured, non multiple of 8 bytes read, data truncated." << std::endl;
      }
      double_cnt = bytes / sizeof(double);
      for ( int i=0; i < double_cnt; ++i ) {
          outputbuffer[i] = (float)inputbuffer[i];
      }
      write(fho, outputbuffer, double_cnt*sizeof(float) );
      if ( loopctr == 1024 ) {
          std::cout << "." << std::flush; // progress indicator
          loopctr = 0;
      } else {
        loopctr++;
      }
    }
    std::cout << "\nConversion of file complete." << std::endl;
    return 0;

}


