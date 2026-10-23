//
// Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
// SPDX-License-Identifier: MIT
//
// Author: Mark Rollins

#pragma once

#include <adf.h>

using namespace adf;

// ------------------------------------------------------------
// DFT-4 compute kernel
// ------------------------------------------------------------
// Direct radix-4 DFT butterfly (no multiplies required - only +-1,+-i
// factors). Two independent streams each carry one 4-pt transform's
// samples per NFFT iterations (SSR=2: 1 sample/cycle/stream).

template<class TT_DATA, unsigned NFFT>
class dft4_compute {
public:
  dft4_compute( void );

  void run( input_stream<TT_DATA>* restrict ss0_i,  input_stream<TT_DATA>* restrict ss1_i,
            output_stream<TT_DATA>* restrict ss0_o, output_stream<TT_DATA>* restrict ss1_o );

  static void registerKernelClass( void )
  {
    REGISTER_FUNCTION(dft4_compute::run);
  }
};
