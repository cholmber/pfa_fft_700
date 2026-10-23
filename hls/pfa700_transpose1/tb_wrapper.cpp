//
// Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
// SPDX-License-Identifier: MIT
//
// Author: Mark Rollins

#include "pfa700_transpose1.h"
#include "pfa700_transpose1_luts.h"
#include <iostream>

using namespace std;
using namespace pfa700_transpose1;

// ------------------------------------------------------------
// Main
// ------------------------------------------------------------
// Ramp test: feed samples whose VALUE equals their own input index
// (0..NFFT-1, repeating), so correctness of the transpose can be
// checked directly against the known address table without needing a
// separate golden-data file.

extern void pfa700_transpose1_wrapper( TT_STREAM sig_i[SSR_IO], TT_STREAM sig_o[SSR_IO] );

static const TT_ADDR wr_addr0[HALF] = WR_ADDR0;
static const TT_ADDR wr_addr1[HALF] = WR_ADDR1;
static const TT_ADDR rd_addr0[HALF] = RD_ADDR0;
static const TT_ADDR rd_addr1[HALF] = RD_ADDR1;

int main( void )
{
  TT_STREAM sig_i[SSR_IO];
  TT_STREAM sig_o[SSR_IO];

  // Exercise ping-pong across a few frames. Output for input frame ff
  // only appears one frame later (once its bank is fully written), so
  // one extra frame of input is needed to flush the last real frame
  // through, and the very first cycle-group's output (reading the
  // uninitialized "other" bank) is garbage and must be discarded.
  static constexpr int NFRAME = 3;

  // Pre-fill all input samples (value == its own flat index) using the
  // same WR convention the kernel uses to reconstruct that index:
  for (int ff=0; ff < NFRAME+1; ff++) {
    for (int pp=0; pp < HALF; pp++) {
      sig_i[0].write( TT_DATA(wr_addr0[pp]) );
      sig_i[1].write( TT_DATA(wr_addr1[pp]) );
    }
  }

  // Run DUT:
  for (int ss=0; ss < HALF*(NFRAME+1); ss++) {
    pfa700_transpose1_wrapper( sig_i, sig_o );
  }

  // Discard the first cycle-group's output (garbage):
  for (int pp=0; pp < HALF; pp++) {
    sig_o[0].read();
    sig_o[1].read();
  }

  // Check the remaining output streams against the known RD_ADDR tables:
  bool flag = 0;
  for (int ff=0; ff < NFRAME; ff++) {
    for (int pp=0; pp < HALF; pp++) {
      TT_DATA act0 = sig_o[0].read();
      TT_DATA act1 = sig_o[1].read();
      TT_DATA gld0 = TT_DATA(rd_addr0[pp]);
      TT_DATA gld1 = TT_DATA(rd_addr1[pp]);
      bool bad0 = ( act0 != gld0 );
      bool bad1 = ( act1 != gld1 );
      flag |= bad0 || bad1;
      if ( bad0 ) cout << "frame " << ff << " pos " << pp << "  gld0: " << gld0 << "  act0: " << act0 << endl;
      if ( bad1 ) cout << "frame " << ff << " pos " << pp << "  gld1: " << gld1 << "  act1: " << act1 << endl;
    }
  }

  if ( flag == 0 ) cout << "--- PASSED ---" << endl;
  else             cout << "*** FAILED ***" << endl;
  return( flag );
}
