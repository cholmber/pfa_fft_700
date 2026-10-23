//
// Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
// SPDX-License-Identifier: MIT
//
// Author: Mark Rollins

#include "pfa700_dma_snk.h"
using namespace pfa700_dma_snk;

// ------------------------------------------------------------
// Stream Capture
// ------------------------------------------------------------

unsigned capture_streams( TT_DATA (&buff)[NSTREAM][DEPTH], TT_STREAM sig_i[NSTREAM],
                          unsigned word_count )
{
  // Streams arrive in round-robin order (ss0: s0 s2 s4 ... - ss1: s1
  // s3 s5 ...), captured into linear-order DDR storage. Only the last
  // DEPTH cycles (one frame) are kept - everything before that is
  // pipeline-fill garbage or an earlier repeated frame:
  unsigned   cnt_cyc = 0;
  TT_ADDR         dd = 0;
  unsigned      base = word_count - DEPTH;
  while (cnt_cyc < word_count) {
#pragma HLS pipeline II=1
    bool   full = (sig_i[0].empty()==0) && (sig_i[1].empty()==0);
    bool capture = (cnt_cyc < base) ? 0 : 1;
    if (full == 1) {
      TT_DATA val0 = sig_i[0].read();
      TT_DATA val1 = sig_i[1].read();
      if (capture == 1) {
        buff[0][dd] = val0;
        buff[1][dd] = val1;
        dd = dd + 1;
      }
      cnt_cyc++;
    }
  }
  return(cnt_cyc);
}

// ------------------------------------------------------------
// Read Buffer
// ------------------------------------------------------------

void read_buffer( TT_DATA mem[NSTREAM*DEPTH], TT_DATA (&buff)[NSTREAM][DEPTH] )
{
  ap_uint<1> ss = 0;
  TT_ADDR dd = 0;
 READ_BUFF: for (int mm=0; mm < DEPTH*NSTREAM; mm++) {
#pragma HLS PIPELINE II=1
    mem[mm] = buff[ss][dd];
    if ( ss == ap_uint<1>(0) ) {
      ss = 1;
    }
    else {
      ss = 0;
      dd = ( dd == TT_ADDR(DEPTH-1) ) ? TT_ADDR(0) : TT_ADDR(dd + 1);
    }
  }
}

// ------------------------------------------------------------
// Wrapper
// ------------------------------------------------------------

unsigned
pfa700_dma_snk_wrapper( pfa700_dma_snk::TT_DATA mem[pfa700_dma_snk::NSTREAM*pfa700_dma_snk::DEPTH],
                        unsigned int word_count,
                        pfa700_dma_snk::TT_STREAM sig_i[pfa700_dma_snk::NSTREAM] )
{
#pragma HLS interface m_axi      port=mem         bundle=gmem    offset=slave   depth=DEPTH*NSTREAM
#pragma HLS interface axis       port=sig_i
#pragma HLS interface s_axilite  port=word_count  bundle=control
#pragma HLS interface s_axilite  port=mem         bundle=control
#pragma HLS interface s_axilite  port=return      bundle=control
#pragma HLS DATAFLOW

  // Internal buffer:
  TT_DATA buff[NSTREAM][DEPTH];
#pragma HLS array_partition variable=buff dim=1
  unsigned cycle_count;

  // Front end load from DDR4 to PL BRAM:
  cycle_count = capture_streams( buff, sig_i, word_count );

  // Back end transmit from PL BRAM to AIE:
  read_buffer( mem, buff );

  return(cycle_count);
}
