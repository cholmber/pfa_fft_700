//
// Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
// SPDX-License-Identifier: MIT
//
// Author: Mark Rollins

#pragma once

#include <complex>
#include <ap_fixed.h>
#include <hls_stream.h>

namespace pfa700_permute_i {
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
// one full NFFT=700 frame (ping-pong) then reads it back out through a
// precomputed permutation LUT (data/compute_perm_3d's P_i, split across
// the 2 output streams to match dft7w's independent-transforms-per-
// stream convention). This routine needs to be II=1.

void pfa700_permute_i_wrapper( pfa700_permute_i::TT_STREAM sig_i[pfa700_permute_i::SSR_IO],
                               pfa700_permute_i::TT_STREAM sig_o[pfa700_permute_i::SSR_IO] );
