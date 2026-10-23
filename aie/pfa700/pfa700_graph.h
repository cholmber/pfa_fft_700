//
// Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
// SPDX-License-Identifier: MIT
//
// Author: Mark Rollins

#pragma once

#include <adf.h>
#include <vector>

#include "dft7_graph.h"
#include "dft25_graph.h"
#include "dft4_graph.h"

using namespace adf;

// ------------------------------------------------------------
// PFA-700 = 7 x 25 x 4 top level graph
// ------------------------------------------------------------
// Each DFT kernel is a single tile (SSR=2 design, no need for the
// original's multi-tile + combiner split), so tile placement here is
// a simple 1x1 per kernel instead of the original's hand-tuned
// multi-tile boxes.

class pfa700_graph : public graph {
public:

  static constexpr int DFT7_X  = 18;
  static constexpr int DFT7_Y  =  0;
  static constexpr int DFT25_X = 19;
  static constexpr int DFT25_Y =  0;
  static constexpr int DFT4_X  = 20;
  static constexpr int DFT4_Y  =  0;

  // Declare vector of PLIO ports:
  std::array< input_plio,2> dft7_i;
  std::array<output_plio,2> dft7_o;
  std::array< input_plio,2> dft25_i;
  std::array<output_plio,2> dft25_o;
  std::array< input_plio,2> dft4_i;
  std::array<output_plio,2> dft4_o;

  // Declare all graphs:
  dft7_graph  dft7;
  dft25_graph dft25;
  dft4_graph  dft4;

  // Graph constructor:
  pfa700_graph( void )
  {
    for (int ii=0; ii < 2; ii++) {
      std::string pname_dft7_i = "PLIO_dft7_i_" + std::to_string(ii);
      std::string pname_dft7_o = "PLIO_dft7_o_" + std::to_string(ii);
      std::string pname_dft25_i = "PLIO_dft25_i_" + std::to_string(ii);
      std::string pname_dft25_o = "PLIO_dft25_o_" + std::to_string(ii);
      std::string pname_dft4_i = "PLIO_dft4_i_" + std::to_string(ii);
      std::string pname_dft4_o = "PLIO_dft4_o_" + std::to_string(ii);
      std::string fname_dft7_i = "data/dft7_i_" + std::to_string(ii) + ".txt";
      std::string fname_dft7_o = "data/dft7_o_" + std::to_string(ii) + ".txt";
      std::string fname_dft25_i = "data/dft25_i_" + std::to_string(ii) + ".txt";
      std::string fname_dft25_o = "data/dft25_o_" + std::to_string(ii) + ".txt";
      std::string fname_dft4_i = "data/dft4_i_" + std::to_string(ii) + ".txt";
      std::string fname_dft4_o = "data/dft4_o_" + std::to_string(ii) + ".txt";
      dft7_i[ii] =  input_plio::create(pname_dft7_i,plio_32_bits,fname_dft7_i);
      dft7_o[ii] = output_plio::create(pname_dft7_o,plio_32_bits,fname_dft7_o);
      dft25_i[ii] =  input_plio::create(pname_dft25_i,plio_32_bits,fname_dft25_i);
      dft25_o[ii] = output_plio::create(pname_dft25_o,plio_32_bits,fname_dft25_o);
      dft4_i[ii] =  input_plio::create(pname_dft4_i,plio_32_bits,fname_dft4_i);
      dft4_o[ii] = output_plio::create(pname_dft4_o,plio_32_bits,fname_dft4_o);

      // Connect inputs and outputs:
      connect<stream,stream>d7i( dft7_i[ii].out[0],  dft7.sig_i[ii] );
      connect<stream,stream>d7o( dft7.sig_o[ii],     dft7_o[ii].in[0] );

      connect<stream,stream>d25i( dft25_i[ii].out[0], dft25.sig_i[ii] );
      connect<stream,stream>d25o( dft25.sig_o[ii],     dft25_o[ii].in[0] );

      connect<stream,stream>d4i( dft4_i[ii].out[0], dft4.sig_i[ii] );
      connect<stream,stream>d4o( dft4.sig_o[ii],    dft4_o[ii].in[0] );

      // Introduce FIFOs to absorb stream stalling on PLIOs using stream connections:
      fifo_depth(d7o) = 256;
      fifo_depth(d25o) = 256;
      fifo_depth(d4o) = 256;
    } // ii

    location<kernel>(dft7.k_tile0)  = tile(DFT7_X,DFT7_Y);
    location<kernel>(dft25.k_tile0) = tile(DFT25_X,DFT25_Y);
    location<kernel>(dft4.k_tile0)  = tile(DFT4_X,DFT4_Y);
  }
};
