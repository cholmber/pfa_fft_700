//
// Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
// SPDX-License-Identifier: MIT
//
// Author: Mark Rollins

#pragma once

#include <adf.h>

using namespace adf;

// ------------------------------------------------------------
// DFT-7 compute kernel (SSR=2: 1 sample/cycle/stream)
// ------------------------------------------------------------
// Two independent streams, each carrying its own sequential run of
// NFFT 7-pt transforms (NFFT = N2*N3 = 25*4 = 100, the other two
// PFA-700 factors). Each transform is computed as a direct 7x7 DFT
// matrix multiply (coefficients from matlab/wfta.m's wfta7 case).

template<class TT_DATA, class TT_TWIDDLE, class TT_ACC, unsigned NFFT>
class dft7_compute {
public:
  static constexpr unsigned NPT = 7;
  static constexpr unsigned DNSHIFT = 15;
  static constexpr unsigned COEFF_DEPTH = NPT*NPT;
private:
  alignas(16) TT_TWIDDLE (&coeff)[COEFF_DEPTH];
public:
  // Constructor:
  dft7_compute( TT_TWIDDLE (&coeff_i)[COEFF_DEPTH] );

  // Run:
  void run( input_stream<TT_DATA>* restrict ss0_i,  input_stream<TT_DATA>* restrict ss1_i,
            output_stream<TT_DATA>* restrict ss0_o, output_stream<TT_DATA>* restrict ss1_o );

  static void registerKernelClass( void )
  {
    REGISTER_FUNCTION( dft7_compute::run );
    REGISTER_PARAMETER( coeff );
  }
};
