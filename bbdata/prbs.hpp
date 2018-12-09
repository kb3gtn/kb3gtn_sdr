#pragma once
#include <iostream>
#include <vector>
#include <cstdint>
#include <cmath>

// PRBS patterns
enum prbs_pattern_t {
  USER = 0,
  ALL_ZEROS = 1,
  ALL_ONES = 2,
  ALT_ONE_ZERO = 3,
  ITU_PN9 = 4,
  ITU_PN11 = 5,
  ITU_PN15 = 6,
  ITU_PN23 = 7
};

// utility methods
int popcount64(uint64_t x);
uint8_t flipbitorder(uint8_t b);
uint64_t TAP(int bit_idx);
void PrintDataBuffer( std::vector<uint8_t> buffer );
void PrintDataBuffer( uint8_t *mem, int len );


// pattern table entry
struct pattern_table_entry_t {
  int pattern_idx;       // Index in table
  uint64_t reg;          // regsiter initial value
  uint64_t reg_mask;     // register active bit mask
  uint64_t fb_mask;      // register feedback bit mask
  uint64_t reg_len_bits; // number of bits in active mask
  char name[32];         // name of this pattern
};

// pre-defined patterns for this generator.
// user defined patters will require manual update to pattern_info_t data member
// in the Handle top setup register taps and length values..
const struct pattern_table_entry_t pattern_lookup_table[] = {
    {USER, 0, 0, 0, 0, "user pattern"},
    {ALL_ZEROS, 0, 0, 0, 1, "all zeros"},
    {ALL_ONES, 1, 1, 1, 1, "all ones"},
    {ALT_ONE_ZERO, 0x2, 0x3, 0x2, 2, "alt one zero" },
    {ITU_PN9, 0x1FF, 0x1FF, TAP(9) | TAP(5), 9, "ITU PN9"},
    {ITU_PN11, 0x7FF, 0x7FF, TAP(11) | TAP(9), 11, "ITU PN11"},
    {ITU_PN15, 0x7FFF, 0x7FFF, TAP(15) | TAP(14), 15, "ITU PN15"},
    {ITU_PN23, 0x7FFFFF, 0x7FFFFF, TAP(23) | TAP(18), 23, "ITU PN23"}
};

// data/state and function for PRBS pattern generation.
struct PRBSGEN {
    // generate PRBS pattern to fill the buffer.
    void generate( std::vector<uint8_t> *buffer );
    void generate( uint8_t *buffer, int len );
    // number of bits sent.
    uint64_t bits_tx;
    // register
    uint64_t reg;
    uint64_t reg_mask;
    uint64_t fb_mask;
    uint64_t reg_len_bits;
    // constructors
    // initialize from known pattern
    PRBSGEN( prbs_pattern_t pat );
    // initialize from specified parameters
    PRBSGEN( uint64_t _reg, uint64_t _reg_mask, uint64_t _fb_mask, uint64_t _reg_len_bits );
};

// data/state and functions for PRBS pattern checking.
struct PRBSCHK {
    // generate PRBS pattern to fill the buffer.
    void check( std::vector<uint8_t> *buffer );
    void check( uint8_t *buffer, int len );
    // number of bits received.
    // number of bits received.
    uint64_t bits_rx;
    uint64_t bits_rx_locked;
    uint64_t bit_errors_detected;
    uint64_t isLocked;
    uint64_t sync_slips;
    double getBER();
    void reset_stats();
    double __bit_match;
    // register
    uint64_t reg;
    uint64_t reg_mask;
    uint64_t fb_mask;
    uint64_t reg_len_bits;
    // constructors
    // initialize from known pattern
    PRBSCHK( prbs_pattern_t pat );
    // initialize from specified parameters
    PRBSCHK( uint64_t _reg, uint64_t _reg_mask, uint64_t _fb_mask, uint64_t _reg_len_bits );
};





