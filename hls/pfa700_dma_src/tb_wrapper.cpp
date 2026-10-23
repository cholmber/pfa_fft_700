//
// Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
// SPDX-License-Identifier: MIT
//
// Author: Mark Rollins

#include "pfa700_dma_src.h"
#include <iostream>

using namespace std;
using namespace pfa700_dma_src;

// ------------------------------------------------------------
// Main
// ------------------------------------------------------------

extern void pfa700_dma_src_wrapper( TT_DATA* mem, int loop_cnt, TT_STREAM sig_o[NSTREAM] );

int main( void )
{
  // Create DDR4 contents (one frame, incrementing ramp):
  TT_DATA ddr4[NSTREAM*DEPTH];
  for (int mm=0; mm < NSTREAM*DEPTH; mm++) {
    ddr4[mm] = TT_DATA(mm);
  }

  // Run DUT:
  int loop_cnt = 3;
  TT_STREAM sig_o[NSTREAM];
  pfa700_dma_src_wrapper( ddr4, loop_cnt, sig_o );

  for (int ss=0; ss < NSTREAM; ss++) {
    cout << "Stream" << ss << " Size: " << sig_o[ss].size() << endl;
  }

  // Check output streams: round-robin order repeated loop_cnt times,
  // followed by LATENCY cycles of zero-padding:
  bool flag = 0;
  for (int ll=0; ll < loop_cnt; ll++) {
    for (int dd=0; dd < DEPTH; dd++) {
      TT_DATA act0 = sig_o[0].read();
      TT_DATA act1 = sig_o[1].read();
      TT_DATA gld0 = TT_DATA(2*dd+0);
      TT_DATA gld1 = TT_DATA(2*dd+1);
      bool bad0 = ( act0 != gld0 );
      bool bad1 = ( act1 != gld1 );
      flag |= bad0 || bad1;
      if ( bad0 ) cout << "ll: " << ll << " dd: " << dd << "  gld0: " << gld0 << "  act0: " << act0 << endl;
      if ( bad1 ) cout << "ll: " << ll << " dd: " << dd << "  gld1: " << gld1 << "  act1: " << act1 << endl;
    }
  }
  for (int dd=0; dd < LATENCY; dd++) {
    TT_DATA act0 = sig_o[0].read();
    TT_DATA act1 = sig_o[1].read();
    bool bad = ( act0 != TT_DATA(0) ) || ( act1 != TT_DATA(0) );
    flag |= bad;
    if ( bad ) cout << "flush dd: " << dd << "  act0: " << act0 << "  act1: " << act1 << endl;
  }

  if ( flag == 0 ) cout << "--- PASSED ---" << endl;
  else             cout << "*** FAILED ***" << endl;
  return( flag );
}
