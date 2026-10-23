//
// Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
// SPDX-License-Identifier: MIT
//
// Author: Mark Rollins

#pragma once

#include <complex>
#include <ap_fixed.h>
#include <hls_stream.h>

namespace pfa700_dma_src {
  static constexpr unsigned      NSTREAM = 2;
  static constexpr unsigned         NFFT = 700;
  static constexpr unsigned        DEPTH = NFFT/NSTREAM; // samples/stream per frame
  static constexpr unsigned        NBITS = 32;           // 1 cint16 sample/word (SSR=2)
  // Pipeline-fill latency of the 4 downstream PL kernels (permute_i,
  // transpose1, transpose2, permute_o), each of which adds one full
  // DEPTH-cycle frame of delay (ping-pong buffering):
  static constexpr unsigned      LATENCY = 4*DEPTH;
  typedef ap_uint<NBITS>                 TT_DATA;
  typedef hls::stream<TT_DATA>           TT_STREAM;
  typedef ap_uint<10>                    TT_ADDR; // # bits = ceil(log2(DEPTH))
};

// Run:
// SSR=2 design: round-robin samples from DDR4 (linear order) across
// the 2 output streams, 1 sample/cycle/stream.
void pfa700_dma_src_wrapper( pfa700_dma_src::TT_DATA mem[pfa700_dma_src::NSTREAM*pfa700_dma_src::DEPTH],
                             int loop_cnt,
                             pfa700_dma_src::TT_STREAM sig_o[pfa700_dma_src::NSTREAM] );
