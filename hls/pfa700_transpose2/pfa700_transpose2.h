//
// Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
// SPDX-License-Identifier: MIT
//
// Author: Mark Rollins

#pragma once

#include <complex>
#include <ap_fixed.h>
#include <hls_stream.h>

namespace pfa700_transpose2 {
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
// one full NFFT=700 frame (ping-pong) of dft25w output then reads it
// back out through a precomputed transpose LUT (compute_addr_3d dim=3)
// to match dft4's split-transform-across-streams convention. This
// routine needs to be II=1.

void pfa700_transpose2_wrapper( pfa700_transpose2::TT_STREAM sig_i[pfa700_transpose2::SSR_IO],
                                pfa700_transpose2::TT_STREAM sig_o[pfa700_transpose2::SSR_IO] );
