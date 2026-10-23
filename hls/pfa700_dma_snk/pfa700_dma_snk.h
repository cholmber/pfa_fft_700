//
// Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
// SPDX-License-Identifier: MIT
//
// Author: Mark Rollins

#pragma once

#include <complex>
#include <ap_fixed.h>
#include <hls_stream.h>

namespace pfa700_dma_snk {
  static constexpr unsigned      NSTREAM = 2;
  static constexpr unsigned         NFFT = 700;
  static constexpr unsigned        DEPTH = NFFT/NSTREAM; // samples/stream per frame
  static constexpr unsigned        NBITS = 32;           // 1 cint16 sample/word (SSR=2)
  // Pipeline-fill latency of the 4 upstream PL kernels (permute_i,
  // transpose1, transpose2, permute_o), each of which adds one full
  // DEPTH-cycle frame of delay (ping-pong buffering) - must match
  // pfa700_dma_src's LATENCY:
  static constexpr unsigned      LATENCY = 4*DEPTH;
  typedef ap_uint<NBITS>                 TT_DATA;
  typedef hls::stream<TT_DATA>           TT_STREAM;
  typedef ap_uint<10>                    TT_ADDR; // # bits = ceil(log2(DEPTH))
};

// Run:
// SSR=2 design: capture 1 sample/cycle/stream, discard the first
// LATENCY cycles (garbage from upstream pipeline fill), then store the
// remaining samples to DDR4 in linear order (inverse of dma_src's
// round-robin split). word_count = total cycles to consume from the
// stream (loop_cnt*DEPTH + LATENCY, matching dma_src's total emission).
unsigned pfa700_dma_snk_wrapper( pfa700_dma_snk::TT_DATA mem[pfa700_dma_snk::NSTREAM*pfa700_dma_snk::DEPTH],
                                 unsigned int word_count,
                                 pfa700_dma_snk::TT_STREAM sig_i[pfa700_dma_snk::NSTREAM] );
