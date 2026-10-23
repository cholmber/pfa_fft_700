//
// Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
// SPDX-License-Identifier: MIT
//
// Author: Mark Rollins

#include "dft4_graph.h"

using namespace adf;

template<int DFT4_X, int DFT4_Y>
class dut_graph : public graph {
  dft4_graph dft4;
public:
  std::array< input_plio,2> sig_i;
  std::array<output_plio,2> sig_o;
  dut_graph( void )
  {
    sig_i[0] =  input_plio::create("PLIO_i_0",plio_64_bits,"data/sig_i_0.txt");
    sig_i[1] =  input_plio::create("PLIO_i_1",plio_64_bits,"data/sig_i_1.txt");
    sig_o[0] = output_plio::create("PLIO_o_0",plio_64_bits,"data/sig_o_0.txt");
    sig_o[1] = output_plio::create("PLIO_o_1",plio_64_bits,"data/sig_o_1.txt");
    connect<stream>(sig_i[0].out[0],dft4.sig_i[0]);
    connect<stream>(sig_i[1].out[0],dft4.sig_i[1]);
    connect<stream>(dft4.sig_o[0],sig_o[0].in[0]);
    connect<stream>(dft4.sig_o[1],sig_o[1].in[0]);

    location<kernel>(dft4.k_tile0) = tile(DFT4_X,DFT4_Y);
  }
};

// Instantiate AIE graph:
dut_graph<18,0> aie_dut;

// Initialize and run the graph:
int main(void)
{
  aie_dut.init();
  aie_dut.run(1);
  aie_dut.end();

  return 0;
}
