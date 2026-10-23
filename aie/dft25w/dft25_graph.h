//
// Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
// SPDX-License-Identifier: MIT
//
// Author: Mark Rollins

#pragma once

#include <adf.h>
#include "dft25_compute.h"
#include "dft25_twiddle.h"

using namespace adf;

// ------------------------------------------------------------
// Top Level Graph
// ------------------------------------------------------------
// Single tile: at SSR=2 there is no need to split work across multiple
// compute tiles + a combiner (as the original 8-wide design did).

class dft25_graph : public graph {
public:
  static constexpr unsigned NFFT = 7*4; // other two factors (N1 x N3)
  typedef cint16 TT_DATA;
  typedef cint16 TT_TWIDDLE;
  typedef int32_t TT_ACC;

  kernel k_tile0;

  std::array<port< input>,2> sig_i;
  std::array<port<output>,2> sig_o;

  using TT_COMPUTE = dft25_compute<TT_DATA,TT_TWIDDLE,TT_ACC,NFFT>;

  dft25_graph( void )
  {
    k_tile0 = kernel::create_object<TT_COMPUTE>( std::vector<TT_TWIDDLE>{DFT25_COEFF} );
    source(k_tile0) = "dft25_compute.cpp";
    runtime<ratio>(k_tile0) = 0.9;

    connect<stream,stream>(sig_i[0],k_tile0.in[0]);
    connect<stream,stream>(sig_i[1],k_tile0.in[1]);
    connect<stream,stream>(k_tile0.out[0],sig_o[0]);
    connect<stream,stream>(k_tile0.out[1],sig_o[1]);
  }
};
