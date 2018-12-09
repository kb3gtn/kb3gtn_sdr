#include <iostream>
#include <fstream>
#include <cstdint>
#include "prbs.hpp"

int main() {
    std::cout << "BERT TEST\n";
    // create a txbert instance
    PRBSGEN txbert( ITU_PN15 );
    // create a rxbert checker
    PRBSCHK rxbert( ITU_PN15 );
    // create a empty databuffer of 32KB
    std::vector<uint8_t> databuffer(32768);
    // fill buffer with PN data
    txbert.generate( &databuffer );
    // print the first 32 bytes of databuffer
    std::cout << "First 32 bytes of data in the buffer...\n";
    PrintDataBuffer( databuffer.data(), 32 );
    std::cout << "----------------------------------------\n";
     // check generated buffer
    rxbert.check( &databuffer );
    // print rxbert status
    std::cout << "BERT Results after 32768 bytes of test data\n";
    std::cout << "Bits RX           " << rxbert.bits_rx << std::endl;
    std::cout << "bits in Lock      " << rxbert.bits_rx_locked << std::endl;
    std::cout << "bit errors        " << rxbert.bit_errors_detected << std::endl;
    std::cout << "Lock State        " << rxbert.isLocked << std::endl;
    std::cout << "sync slips        " << rxbert.sync_slips << std::endl;
    std::cout << "BER               " << rxbert.getBER() << std::endl;
    std::cout << "-----------------------------------------\n";
    return 0;
}

