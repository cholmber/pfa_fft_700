//
// Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
// SPDX-License-Identifier: MIT
//
// Author: Mark Rollins

#include "pfa700_transpose1.h"
#include "pfa700_transpose1_luts.h"

using namespace pfa700_transpose1;

// ------------------------------------------------------------
// Wrapper
// ------------------------------------------------------------
// Buffers a full NFFT-sample frame (ping-pong across 2 banks) and reads
// it back out through a precomputed per-stream address LUT. 2 samples
// move per cycle (1 in + 1 out per stream), matching the HALF=NFFT/2
// cycles/frame needed at SSR=2.

void pfa700_transpose1_wrapper( TT_STREAM sig_i[SSR_IO], TT_STREAM sig_o[SSR_IO] )
{
#pragma HLS interface mode=ap_ctrl_none port=return
#pragma HLS pipeline II=1

  static bool    ping = 0;
  static TT_ADDR pos  = 0;
#pragma HLS reset variable=ping
#pragma HLS reset variable=pos

  static TT_DATA buff[2][NFFT];
#pragma HLS array_partition variable=buff dim=1
#pragma HLS bind_storage variable=buff type=RAM_T2P impl=bram
#pragma HLS dependence variable=buff type=intra false

  static const TT_ADDR wr_addr0[HALF] = WR_ADDR0;
  static const TT_ADDR wr_addr1[HALF] = WR_ADDR1;
  static const TT_ADDR rd_addr0[HALF] = RD_ADDR0;
  static const TT_ADDR rd_addr1[HALF] = RD_ADDR1;

  // Write incoming samples into the "ping" bank:
  buff[ping][wr_addr0[pos]] = sig_i[0].read();
  buff[ping][wr_addr1[pos]] = sig_i[1].read();

  // Read transposed samples out of the "!ping" bank (prior frame):
  sig_o[0].write( buff[!ping][rd_addr0[pos]] );
  sig_o[1].write( buff[!ping][rd_addr1[pos]] );

  // Update state:
  bool last = ( pos == TT_ADDR(HALF-1) );
  ping = ( last == 1 ) ? !ping     : ping;
  pos  = ( last == 1 ) ? TT_ADDR(0) : TT_ADDR(pos+1);
}
