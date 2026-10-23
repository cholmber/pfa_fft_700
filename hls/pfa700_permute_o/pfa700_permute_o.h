//
// Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
// SPDX-License-Identifier: MIT
//
// Author: Mark Rollins

#pragma once

#include <complex>
#include <ap_fixed.h>
#include <hls_stream.h>

namespace pfa700_permute_o {
  static constexpr unsigned  SSR_IO = 2;
  static constexpr unsigned    NFFT = 7 * 25 * 4;
  static constexpr unsigned    HALF = NFFT/2;
  static constexpr unsigned   NBITS = 32;
  typedef ap_uint<NBITS>             TT_DATA;
  typedef hls::stream<TT_DATA>       TT_STREAM;
  typedef ap_uint<10>                TT_ADDR; // # bits = ceil(log2(NFFT)) = 10
};

// Run:
// SSR=2 design: 1 sample/cycle/stream (2 samples/cycle total). Buffers
// one full NFFT=700 frame (ping-pong) of dft4 output then reads it back
// out through a precomputed inverse-P_o LUT, producing the final
// output in simple round-robin order for dma_snk. This routine needs
// to be II=1.

void pfa700_permute_o_wrapper( pfa700_permute_o::TT_STREAM sig_i[pfa700_permute_o::SSR_IO],
                               pfa700_permute_o::TT_STREAM sig_o[pfa700_permute_o::SSR_IO] );
