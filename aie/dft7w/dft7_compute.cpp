//
// Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
// SPDX-License-Identifier: MIT
//
// Author: Mark Rollins

#include <adf.h>

#include "dft7_compute.h"

// ------------------------------------------------------------
// Complex multiply-accumulate helper (32-bit accumulator, Q15 inputs)
// ------------------------------------------------------------

template<class TT_DATA>
static inline void cmac( int32_t &acc_re, int32_t &acc_im, TT_DATA a, TT_DATA b )
{
  acc_re += (int32_t)a.real*(int32_t)b.real - (int32_t)a.imag*(int32_t)b.imag;
  acc_im += (int32_t)a.real*(int32_t)b.imag + (int32_t)a.imag*(int32_t)b.real;
}

// Round + downshift (Q15 x Q15 = Q30 -> Q15) + saturate to cint16:
template<class TT_DATA, unsigned SHIFT>
static inline TT_DATA round_sat( int32_t re, int32_t im )
{
  const int32_t half = (int32_t(1) << (SHIFT-1));
  int32_t r = (re + half) >> SHIFT;
  int32_t i = (im + half) >> SHIFT;
  if (r >  32767) r =  32767;
  if (r < -32768) r = -32768;
  if (i >  32767) i =  32767;
  if (i < -32768) i = -32768;
  TT_DATA out;
  out.real = (int16_t)r;
  out.imag = (int16_t)i;
  return out;
}

// ------------------------------------------------------------
// Constructor
// ------------------------------------------------------------

template<class TT_DATA, class TT_TWIDDLE, class TT_ACC, unsigned NFFT>
dft7_compute<TT_DATA,TT_TWIDDLE,TT_ACC,NFFT>::dft7_compute( TT_TWIDDLE (&coeff_i)[COEFF_DEPTH] )
: coeff(coeff_i)
{
}

// ------------------------------------------------------------
// Run
// ------------------------------------------------------------

template<class TT_DATA, class TT_TWIDDLE, class TT_ACC, unsigned NFFT>
void dft7_compute<TT_DATA,TT_TWIDDLE,TT_ACC,NFFT>
::run( input_stream<TT_DATA>* restrict ss0_i,  input_stream<TT_DATA>* restrict ss1_i,
       output_stream<TT_DATA>* restrict ss0_o, output_stream<TT_DATA>* restrict ss1_o )
{
  for (unsigned rr=0; rr < NFFT/2; rr++)
    chess_loop_range(NFFT/2,)
    chess_prepare_for_pipelining
  {
    TT_DATA buf0[NPT], buf1[NPT];
    for (unsigned n=0; n < NPT; n++) {
      buf0[n] = readincr(ss0_i);
      buf1[n] = readincr(ss1_i);
    }
    for (unsigned k=0; k < NPT; k++) {
      int32_t acc0_re=0, acc0_im=0, acc1_re=0, acc1_im=0;
      for (unsigned n=0; n < NPT; n++) {
        TT_TWIDDLE c = coeff[k*NPT+n];
        cmac(acc0_re,acc0_im,buf0[n],c);
        cmac(acc1_re,acc1_im,buf1[n],c);
      }
      writeincr(ss0_o, round_sat<TT_DATA,DNSHIFT>(acc0_re,acc0_im));
      writeincr(ss1_o, round_sat<TT_DATA,DNSHIFT>(acc1_re,acc1_im));
    }
  }
}
