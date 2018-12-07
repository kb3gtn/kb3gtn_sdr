/* libPRBS.h
 *
 *
 */

#pragma once

#include <cstdint>
#include <cstdio>
#include <cstdlib>

namespace PRBS {
enum bit_order_t { MSB_ORDER, LSB_ORDER };

// should match index in pattern table..
enum pattern_t {
  USER = 0,
  ALL_ZEROS = 1,
  ALL_ONES = 2,
  ITU_PN9 = 3,
  ITU_PN11 = 4,
  ITU_PN15 = 5,
  ITU_PN23 = 6
};

// convert enum to string
const char *toString(pattern_t);

/*
struct pattern_info_t {
  uint64_t reg;      // shiftregister initial value
  uint64_t reg_mask; // mask of valid bits in regsiter
  uint64_t fb_mask;  // bit mask of feedback bits
  int reg_len_bits;  // number of bits in register
};
*/

// map lfsr taps to register bits, off by 1
uint64_t TAP(int bit_idx);

struct pattern_table_entry_t {
  int pattern_idx;       // Index in table
  uint64_t reg;          // regsiter initial value
  uint64_t reg_mask;     // register active bit mask
  uint64_t fb_mask;      // register feedback bit mask
  uint64_t reg_len_bits; // number of bits in active mask
  char name[32];         // name of this pattern
};

enum bert_state { LOCKED, SEARCH };

struct bert_status {
  bert_status();                // initialization structure (zero out)
  bert_state state;             // locked / searching
  uint64_t bits_rx;             // total bits received
  uint64_t bits_rx_locked;      // bits received while locked
  uint64_t bit_errors_detected; // bit errors detected
  double getBER();              // returns BER based on data above
  void reset_stats();           // zeros out all counts above.
};

// pre-defined patterns for this generator.
// user defined patters will require manual update to pattern_info_t data member
// in the Handle top setup register taps and length values..
const struct pattern_table_entry_t pattern_lookup_table[] = {
    {USER, 0, 0, 0, 0, "user pattern"},
    {ALL_ZEROS, 0, 0, 0, 1, "all zeros"},
    {ALL_ONES, 1, 1, 1, 1, "all ones"},
    {ITU_PN9, 0x1FF, 0x1FF, TAP(9) | TAP(5), 9, "ITU PN9"},
    {ITU_PN11, 0x7FF, 0x7FF, TAP(11) | TAP(9), 11, "ITU PN11"},
    {ITU_PN15, 0x7FFF, 0x7FFF, TAP(15) | TAP(14), 15, "ITU PN15"},
    {ITU_PN23, 0x7FFFFF, 0x7FFFFF, TAP(23) | TAP(18), 23, "ITU PN23"}};

struct Handle {
  Handle(pattern_t _pat, bit_order_t _order = MSB_ORDER);
  bit_order_t order;
  pattern_table_entry_t data;
};

// Fill buffer with pattern
void generate_pattern(Handle h, uint8_t *buffer, uint len);
// Check a buffer of pattern
void bert_check(Handle h, bert_status s, uint8_t *buffer, uint len);

// internal methods
int popcount64(uint64_t x);
uint8_t flipbitorder(uint8_t b);

} // namespace PRBS

