//
// Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
// SPDX-License-Identifier: MIT
//
// Author: Mark Rollins

#include "pfa700_dma_snk.h"
#include <iostream>

using namespace std;
using namespace pfa700_dma_snk;

// ------------------------------------------------------------
// Main
// ------------------------------------------------------------

extern unsigned pfa700_dma_snk_wrapper( TT_DATA* mem, unsigned int word_count, TT_STREAM sig_i[NSTREAM] );

int main( void )
{
  TT_STREAM sig_i[NSTREAM];
  TT_DATA mem[NSTREAM*DEPTH];

  static constexpr int LOOP_CNT = 3;
  unsigned word_count = LOOP_CNT*DEPTH + LATENCY;

  // Feed LATENCY cycles of garbage, then LOOP_CNT repeated ramp frames
  // (round-robin order, matching pfa700_dma_src's output convention):
  for (unsigned dd=0; dd < LATENCY; dd++) {
    sig_i[0].write( TT_DATA(0xDEAD) );
    sig_i[1].write( TT_DATA(0xBEEF) );
  }
  for (int ll=0; ll < LOOP_CNT; ll++) {
    for (unsigned dd=0; dd < DEPTH; dd++) {
      sig_i[0].write( TT_DATA(2*dd+0) );
      sig_i[1].write( TT_DATA(2*dd+1) );
    }
  }

  // Run DUT:
  unsigned cycle_count = pfa700_dma_snk_wrapper( mem, word_count, sig_i );
  cout << "cycle_count: " << cycle_count << endl;

  // Check captured frame (only the last frame is kept, in linear order):
  bool flag = 0;
  for (unsigned mm=0; mm < NSTREAM*DEPTH; mm++) {
    TT_DATA gld = TT_DATA(mm);
    bool bad = ( mem[mm] != gld );
    flag |= bad;
    if ( bad ) cout << "mm: " << mm << "  gld: " << gld << "  act: " << mem[mm] << endl;
  }

  if ( flag == 0 ) cout << "--- PASSED ---" << endl;
  else             cout << "*** FAILED ***" << endl;
  return( flag );
}
