//
// Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
// SPDX-License-Identifier: MIT
//
// Author: Mark Rollins

#pragma once

#include <adf.h>
#include "dft4_compute.h"

using namespace adf;

// ------------------------------------------------------------
// Top Level Graph
// ------------------------------------------------------------
// Single tile: a 4-pt DFT is just a radix-4 butterfly (no coefficient
// table needed), so unlike dft7/dft25 there is no separate combiner.

class dft4_graph : public graph {
public:
  static constexpr unsigned NFFT = 7*25; // other two factors (N1 x N2)
  typedef cint16 TT_DATA;

  kernel k_tile0;

  std::array<port< input>,2> sig_i;
  std::array<port<output>,2> sig_o;

  using TT_COMPUTE = dft4_compute<TT_DATA,NFFT>;

  dft4_graph( void )
  {
    k_tile0 = kernel::create_object<TT_COMPUTE>();
    source(k_tile0) = "dft4_compute.cpp";
    runtime<ratio>(k_tile0) = 0.9;

    connect<stream,stream>(sig_i[0],k_tile0.in[0]);
    connect<stream,stream>(sig_i[1],k_tile0.in[1]);
    connect<stream,stream>(k_tile0.out[0],sig_o[0]);
    connect<stream,stream>(k_tile0.out[1],sig_o[1]);
  }
};
