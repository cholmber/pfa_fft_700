//
// Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
// SPDX-License-Identifier: MIT
//
// Author: Mark Rollins

#include <adf.h>

#include "dft4_compute.h"

// ------------------------------------------------------------
// Complex helpers (plain radix-4 butterfly: only +-1, +-i factors,
// so no actual multiplies are required)
// ------------------------------------------------------------

template<class TT_DATA>
static inline TT_DATA cadd( TT_DATA a, TT_DATA b )
{
  TT_DATA r;
  r.real = a.real + b.real;
  r.imag = a.imag + b.imag;
  return r;
}

template<class TT_DATA>
static inline TT_DATA csub( TT_DATA a, TT_DATA b )
{
  TT_DATA r;
  r.real = a.real - b.real;
  r.imag = a.imag - b.imag;
  return r;
}

// Multiply by -j: (re,im) * (-j) = (im,-re)
template<class TT_DATA>
static inline TT_DATA cmulmj( TT_DATA a )
{
  TT_DATA r;
  r.real =  a.imag;
  r.imag = -a.real;
  return r;
}

// ------------------------------------------------------------
// Constructor
// ------------------------------------------------------------

template<class TT_DATA, unsigned NFFT>
dft4_compute<TT_DATA,NFFT>::dft4_compute( void )
{
}

// ------------------------------------------------------------
// Run
// ------------------------------------------------------------
// NFFT (= N1*N2, the other two PFA-700 factors) is odd (7*25=175), so
// unlike dft7/dft25 the 2 streams cannot each carry independent whole
// transforms (that requires an even transform count to alternate).
// Instead, following the same convention as the original design's
// power-of-2 kernel, a SINGLE transform is split across both streams:
// ss0 carries the even-indexed input samples (x0,x2), ss1 carries the
// odd-indexed samples (x1,x3); outputs are split the same way
// (ss0: X0,X2 - ss1: X1,X3).
//
// X0 = x0+x1+x2+x3
// X1 = (x0-x2) - j*(x1-x3)
// X2 = x0-x1+x2-x3
// X3 = (x0-x2) + j*(x1-x3)

template<class TT_DATA, unsigned NFFT>
void dft4_compute<TT_DATA,NFFT>
::run( input_stream<TT_DATA>* restrict ss0_i,  input_stream<TT_DATA>* restrict ss1_i,
       output_stream<TT_DATA>* restrict ss0_o, output_stream<TT_DATA>* restrict ss1_o )
{
  for (unsigned rr=0; rr < NFFT; rr++)
    chess_loop_range(NFFT,)
    chess_prepare_for_pipelining
  {
    TT_DATA x0 = readincr(ss0_i);
    TT_DATA x2 = readincr(ss0_i);
    TT_DATA x1 = readincr(ss1_i);
    TT_DATA x3 = readincr(ss1_i);

    TT_DATA a = cadd(x0,x2);
    TT_DATA b = csub(x0,x2);
    TT_DATA c = cadd(x1,x3);
    TT_DATA d = cmulmj(csub(x1,x3));

    TT_DATA X0 = cadd(a,c);
    TT_DATA X1 = cadd(b,d);
    TT_DATA X2 = csub(a,c);
    TT_DATA X3 = csub(b,d);

    writeincr(ss0_o, X0);
    writeincr(ss0_o, X2);
    writeincr(ss1_o, X1);
    writeincr(ss1_o, X3);
  }
}
