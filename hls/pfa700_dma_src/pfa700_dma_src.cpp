//
// Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
// SPDX-License-Identifier: MIT
//
// Author: Mark Rollins

#include "pfa700_dma_src.h"
using namespace pfa700_dma_src;

// ------------------------------------------------------------
// Load Buffer
// ------------------------------------------------------------

void load_buffer( TT_DATA mem[NSTREAM*DEPTH], TT_DATA (&buff)[NSTREAM][DEPTH] )
{
  // DDR4 holds one frame in linear sample order; round-robin split it
  // across the 2 streams (ss0: s0 s2 s4 ... - ss1: s1 s3 s5 ...):
  bool ss = 0;
  TT_ADDR dd = TT_ADDR(0);
 LOAD_BUFF: for (int mm=0; mm < DEPTH*NSTREAM; mm++) {
#pragma HLS PIPELINE II=1
    buff[ss][dd] = mem[mm];
    if ( ss == 1 ) {
      ss = 0;
      dd = ( dd == TT_ADDR(DEPTH-1) ) ? TT_ADDR(0) : TT_ADDR(dd+1);
    }
    else {
      ss = 1;
    }
  } // mm
}

// ------------------------------------------------------------
// Transmit
// ------------------------------------------------------------

void transmit( TT_DATA (&buff)[NSTREAM][DEPTH], TT_STREAM sig_o[NSTREAM], const int& loop_cnt )
{
 REPEAT: for (int ll=0; ll < loop_cnt; ll++) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=8
  RUN_DEPTH: for (int dd=0; dd < DEPTH; dd++) {
#pragma HLS PIPELINE II=1
      sig_o[0].write( buff[0][dd] );
      sig_o[1].write( buff[1][dd] );
    } // dd
  } // ll
  // Flush the downstream PL pipeline (4 stages x DEPTH cycles of
  // ping-pong latency) so the last real frame is fully delivered:
 FLUSH: for (int dd=0; dd < LATENCY; dd++) {
#pragma HLS PIPELINE II=1
    sig_o[0].write( TT_DATA(0) );
    sig_o[1].write( TT_DATA(0) );
  }
}

// ------------------------------------------------------------
// Wrapper
// ------------------------------------------------------------

void
pfa700_dma_src_wrapper( pfa700_dma_src::TT_DATA mem[pfa700_dma_src::NSTREAM*pfa700_dma_src::DEPTH],
                        int loop_cnt,
                        pfa700_dma_src::TT_STREAM sig_o[pfa700_dma_src::NSTREAM] )
{
#pragma HLS interface m_axi      port=mem         bundle=gmem    offset=slave   depth=DEPTH*NSTREAM
#pragma HLS interface axis       port=sig_o
#pragma HLS interface s_axilite  port=loop_cnt    bundle=control
#pragma HLS interface s_axilite  port=mem         bundle=control
#pragma HLS interface s_axilite  port=return      bundle=control
#pragma HLS DATAFLOW

  // Internal buffer:
  TT_DATA buff[NSTREAM][DEPTH];
#pragma HLS array_partition variable=buff dim=1

  // Front end load from DDR4 to PL BRAM:
  load_buffer( mem, buff );

  // Back end transmit from PL BRAM to AIE:
  transmit( buff, sig_o, loop_cnt );
}
