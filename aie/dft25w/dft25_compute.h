//
// Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
// SPDX-License-Identifier: MIT
//
// Author: Mark Rollins

#pragma once

#include <adf.h>

using namespace adf;

// ------------------------------------------------------------
// DFT-25 compute kernel (SSR=2: 1 sample/cycle/stream)
// ------------------------------------------------------------
// Two independent streams, each carrying its own sequential run of
// NFFT 25-pt transforms (NFFT = N1*N3 = 7*4 = 28, the other two
// PFA-700 factors). Each transform is computed as a direct 25x25 DFT
// matrix multiply (coefficients from matlab/wfta.m's wfta25 case - a
// direct matrix since 25=5^2 is not a product of coprime factors).

template<class TT_DATA, class TT_TWIDDLE, class TT_ACC, unsigned NFFT>
class dft25_compute {
public:
  static constexpr unsigned NPT = 25;
  static constexpr unsigned DNSHIFT = 15;
  static constexpr unsigned COEFF_DEPTH = NPT*NPT;
private:
  alignas(16) TT_TWIDDLE (&coeff)[COEFF_DEPTH];
public:
  // Constructor:
  dft25_compute( TT_TWIDDLE (&coeff_i)[COEFF_DEPTH] );

  // Run:
  void run( input_stream<TT_DATA>* restrict ss0_i,  input_stream<TT_DATA>* restrict ss1_i,
            output_stream<TT_DATA>* restrict ss0_o, output_stream<TT_DATA>* restrict ss1_o );

  static void registerKernelClass( void )
  {
    REGISTER_FUNCTION( dft25_compute::run );
    REGISTER_PARAMETER( coeff );
  }
};
