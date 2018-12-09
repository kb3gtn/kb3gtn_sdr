#include "prbs.hpp"
#include <cmath>

// utility methods

// __INTERNAL__
// returns number of 1's set in a 64 bit number
int popcount64(uint64_t y) {
  y -= ((y >> 1) & 0x5555555555555555ull);
  y = (y & 0x3333333333333333ull) + (y >> 2 & 0x3333333333333333ull);
  int r = ((y + (y >> 4)) & 0xf0f0f0f0f0f0f0full) * 0x101010101010101ull >> 56;
  return r;
}

// __INTERNAL__
// flip order of bits in a byte
// bit 7 -> bit 0
// bit 5 -> bit 2
uint8_t flipbitorder(uint8_t b) {
  b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
  b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
  b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
  return b;
}

uint64_t TAP(int bit_idx) { return (1 << bit_idx - 1); }

PRBSGEN::PRBSGEN( prbs_pattern_t pat ) {
    pattern_table_entry_t e = pattern_lookup_table[ pat ];
    bits_tx = 0;
    reg = e.reg;
    reg_mask = e.reg_mask;
    fb_mask = e.fb_mask;
    reg_len_bits = e.reg_len_bits;
}

PRBSGEN::PRBSGEN( uint64_t _reg, uint64_t _reg_mask, uint64_t _fb_mask, uint64_t _reg_len_bits ) {
    bits_tx = 0;
    reg = _reg;
    reg_mask = _reg_mask;
    fb_mask = _fb_mask;
    reg_len_bits = _reg_len_bits;
}

void PRBSGEN::generate( std::vector<uint8_t> *buffer ) {
    // get pointer data for buffer, call pointer version..
    generate( (uint8_t*)buffer->data(), (int)buffer->size() );
}

void PRBSGEN::generate( uint8_t *buffer, int len) {
  uint8_t *bptr;
  bptr = buffer; // initialize to buffer ptr
  uint8_t wb;    // working byte
  for (int idx = 0; idx < len; ++idx) {
    // for each byte in buffer compute BER Pattern for 8 bits.
    wb = 0;
    for (int bitidx = 0; bitidx < 8; ++bitidx) {
      // for each bit in byte..
      // shift up wb 1 bit
      wb = wb << 1;

      // add top most bit from shiftregisters.
      wb = wb | ((reg & (1 << reg_len_bits - 1)) >> reg_len_bits - 1);

      // get feedback bits to compute next lfsr input.
      uint64_t fb = reg & fb_mask; // select feedback bits in register

      // shift register up 1 bit
      reg = reg << 1; // lsb of regsiter is zero.

      // figure out how many '1's are set in fb
      int bits_set = popcount64(fb);

      // figure out feedback based on XOR of all figure.
      if (bits_set % 2 == 1) {
        // ODD number of bits set should XOR to a '1', and '0' when even.
        reg = reg | 0x1; // set feedback bit from zero to a 1.
      }
      // mask register to remove shifted out bits..
      reg = reg & reg_mask;
    }
    *bptr = wb; // write wb to current buffer location.
    ++bptr;     // point to next buffer location.
  }
  // done.
}



PRBSCHK::PRBSCHK( prbs_pattern_t pat ) {
    pattern_table_entry_t e = pattern_lookup_table[ pat ];
    bits_rx = 0;
    bits_rx_locked = 0;
    bit_errors_detected = 0;
    reg = e.reg;
    reg_mask = e.reg_mask;
    fb_mask = e.fb_mask;
    reg_len_bits = e.reg_len_bits;
    isLocked = 0;
    sync_slips = 0;
    __bit_match = 0;
}

PRBSCHK::PRBSCHK( uint64_t _reg, uint64_t _reg_mask, uint64_t _fb_mask, uint64_t _reg_len_bits ) {
    bits_rx = 0;
    bits_rx_locked = 0;
    bit_errors_detected = 0;
    reg = _reg;
    reg_mask = _reg_mask;
    fb_mask = _fb_mask;
    reg_len_bits = _reg_len_bits;
    isLocked = 0;
    sync_slips = 0;
    __bit_match = 0;
}

void PRBSCHK::check( std::vector<uint8_t> *buffer ) {
    // get pointer data for buffer, call pointer version..
    check( (uint8_t*)buffer->data(), (int)buffer->size() );
}

void PRBSCHK::check( uint8_t *buffer, int len) {
  uint8_t *bptr;
  bptr = buffer; // initialize to buffer ptr
  uint8_t bitin=0;  // working bit.
  uint8_t fb_bit=0; // register tap feedback value.
  for (int idx = 0; idx < len; ++idx) {
    // for each byte in buffer check BER Pattern for 8 bits.
    for (int bitidx = 0; bitidx < 8; ++bitidx) {

      // increment bit counter
      bits_rx++;

      // get input bit.
      // mask for working bit input
      if ( *bptr & ( 1 << (7-bitidx) ) ) {
        bitin = 1;
      } else {
        bitin = 0;
      }

      // compute feedback for next input bit.
      // get feedback bits.
      uint64_t fb = reg & fb_mask; // select feedback bits in registers
      //std::cout << "fb: " << (int)fb << " ";
      // figure out how many '1's are set in fb
      int bits_set = popcount64(fb);
      // figure out feedback result bit.
      if (bits_set % 2 == 1) {
        // ODD number of bits set should XOR to a '1', and '0' when even.
        fb_bit = 0x1; // set feedback bit from zero to a 1.
      } else {
        fb_bit = 0x0; // feedback is a zero.
      }

      // shift register up 1 bit
      reg = reg << 1; // lsb of regsiter to add select feedback into LSB.

      // depending if we are locked to pattern or searching for sync..
      if ( isLocked == 1 ) {
        bits_rx_locked++;
        // when locked feed in the feedback from the register
        reg = reg + fb_bit;
        // test to see if generator output matches input bit.
        if ( bitin == fb_bit ) {
          if ( __bit_match > 0 ) {
            __bit_match--; // decrement error threshold
          }
        } else {
          __bit_match++; // increment error threshold
          bit_errors_detected++; // increment bit errors
        }
        // test to see if we are still in lock
        if ( __bit_match > (reg_len_bits*4) ) {
            // no longer in lock..
            __bit_match = 0;
            isLocked = 0;
            sync_slips++;
        }
      } else {
        reg = reg + bitin;
        // test to see if generator matches input bit.
        if ( bitin == fb_bit ) {
          __bit_match++; // increment __bit_match
        } else {
          __bit_match = 0; // reset __bit_match
        }
        // test to see if we have achived locked
        if ( __bit_match > (reg_len_bits*4)) {
            __bit_match = 0;
            isLocked = 1;
        }
      }

      // mask register to remove shifted out bits..
      reg = reg & reg_mask;
    }
    ++bptr;     // point to next buffer location.
  }
  // done.
}

double PRBSCHK::getBER() {
    if ( bits_rx_locked == 0 ) {
        return NAN;
    }
    return (double)bit_errors_detected / (double)bits_rx_locked;
}

void PRBSCHK::reset_stats() {
    bits_rx = 0;
    bits_rx_locked = 0;
    bit_errors_detected = 0;
    isLocked = 0;
    sync_slips = 0;
    __bit_match = 0;
}


// Print buffer contents as a hex dump display to the screen.
// print std::vector<uint8_t> contents
void PrintDataBuffer( std::vector<uint8_t> buffer ) {
    uint8_t *mem= buffer.data();
    int len = buffer.size();
    PrintDataBuffer(mem,len);
}

// print buffer using a c-ptr and a number of bytes
void PrintDataBuffer( uint8_t *mem, int len ) {
    unsigned int i,j;
    for(i = 0; i < len + ((len % 32) ? (32 - len % 32) : 0); i++) {
        if(i % 32 == 0) {
            printf("0x%06x: ", i);
        }
        if(i < len) {
            printf("%02x ", 0xFF & ((char*)mem)[i]);
        } else {
            printf("   ");
        }
        if(i % 32 == (32 - 1)) {
            for(j = i - (31); j <= i; j++) {
                if(j >= len) {
                     putchar(' ');
                }
                else if(isprint(((char*)mem)[j])) {
                    putchar(0xFF & ((char*)mem)[j]);
                }
                else {
                    putchar('.');
                }
            }
            putchar('\n');
        }
    }
}
