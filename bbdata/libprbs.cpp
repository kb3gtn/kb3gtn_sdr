#include "libprbs.hpp"
#include <stdarg.h>
#include <string.h> // memcpy

using namespace PRBS;

#define DEBUG 1

// debug
void debug(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vfprintf(stdout, fmt, args);
  va_end(args);
}

// returns number of 1's set in a 64 bit number
int PRBS::popcount64(uint64_t y) {
  debug(" popcount(%lX)=", y);
  y -= ((y >> 1) & 0x5555555555555555ull);
  y = (y & 0x3333333333333333ull) + (y >> 2 & 0x3333333333333333ull);
  int r = ((y + (y >> 4)) & 0xf0f0f0f0f0f0f0full) * 0x101010101010101ull >> 56;
  debug("%d ", r);
  return r;
}

// __INTERNAL__
// flip order of bits in a byte
// bit 7 -> bit 0
// bit 5 -> bit 2
uint8_t PRBS::flipbitorder(uint8_t b) {
  b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
  b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
  b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
  return b;
}

const char *PRBS::toString(PRBS::pattern_t p) {
  switch (p) {
  case USER:
    return (const char *)"USER";
  case ALL_ZEROS:
    return (const char *)"ALL ZEROS";
  case ALL_ONES:
    return (const char *)"ALL ONES";
  case ITU_PN9:
    return (const char *)"ITU PN9";
  case ITU_PN11:
    return (const char *)"ITU PN11";
  case ITU_PN15:
    return (const char *)"ITU PN15";
  case ITU_PN23:
    return (const char *)"ITU PN23";
  default:
    return (const char *)"UNKNOWN";
  }
}

uint64_t PRBS::TAP(int bit_idx) { return (1 << bit_idx - 1); }

// initalize internal data structures for the pattern generator.
PRBS::Handle::Handle(pattern_t _pat, bit_order_t _order) {
  order = order;
  // copy record from lookup table to local struct
  data = pattern_lookup_table[_pat];
  debug("Pattern Initialization:\n");
  debug("data.pattern_idx = %d\n", data.pattern_idx);
  debug("data.reg = %lX\n", data.reg);
  debug("data.reg_mask = %lX\n", data.reg_mask);
  debug("data.fb_mask = %lX\n", data.fb_mask);
  debug("data.reg_len_bits =%d\n", data.reg_len_bits);
  debug("data.name = %s\n", data.name);
}

void PRBS::generate_pattern(Handle h, uint8_t *buffer, uint len) {
  uint8_t *bptr;
  bptr = buffer; // initialize to buffer ptr
  uint8_t wb;    // working byte
  for (int idx = 0; idx < len; ++idx) {
    debug("\nStart Byte %d\n", idx);
    debug("---------------------------------\n");

    // for each byte in buffer compute BER Pattern for 8 bits.
    wb = 0;
    for (int bitidx = 0; bitidx < 8; ++bitidx) {
      debug("Bit IDX = %d, lfsr = %08lX  --  ", bitidx, h.data.reg);
      // for each bit in byte..
      // shift up wb 1 bit
      wb = wb << 1;

      // add top most bit from shiftregisters.
      wb = wb | ((h.data.reg & (1 << h.data.reg_len_bits - 1)) >>
                 h.data.reg_len_bits - 1);

      // get feedback bits to compute next lfsr input.
      uint64_t fb =
          h.data.reg & h.data.fb_mask; // select feedback bits in register

      // shift register up 1 bit
      h.data.reg = h.data.reg << 1; // lsb of regsiter is zero.

      // figure out how many '1's are set in fb
      int bits_set = popcount64(fb);

      // figure out feedback based on XOR of all figure.
      if (bits_set % 2 == 1) {
        // ODD number of bits set should XOR to a '1', and '0' when even.
        h.data.reg = h.data.reg | 0x1; // set feedback bit from zero to a 1.
      }
      debug("fb_mask = %08lX, bits_set = %d, fb_bit = %d, wb=%02X\n", fb,
            bits_set, (bits_set % 2), wb);
      // mask register to remove shifted out bits..
      h.data.reg = h.data.reg & h.data.reg_mask;
    }
    *bptr = wb; // write wb to current buffer location.
    ++bptr;     // point to next buffer location.
  }
  // done.
}

void PRBS::bert_check(Handle h, bert_status s, uint8_t *buffer, uint len) {}

