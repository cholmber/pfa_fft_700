%
% Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
% SPDX-License-Identifier: MIT
%
% Author: Mark Rollins

clear all;
close all;

% ------------------------------------------------------------
% Generate Model
% ------------------------------------------------------------

Nports = 2;
Nfft = 7;
Ntransform = 25*4; % other two PFA-700 factors (N2 x N3)
backoff_dB = -15;                       % To achieve (-1,+1) range on AWGN signal

TT = numerictype(1,16,15);
FF = fimath('RoundingMethod','Round','OverflowAction','Saturate');

% Input signal:
taps = firgr('minevn',[0,0.3,0.4,1.0],[1,1,0,0],[1e-4,1e-4]);
fir_i = sqrt(0.5)*10^(0.05*backoff_dB)*complex(randn(1,Nfft*Ntransform),randn(1,Nfft*Ntransform));
fir_o = filter(taps,1,fir_i);
scale = 1/32;
sig_i = fi(reshape(scale*fir_o,Nfft,Ntransform),TT,FF);

% Output:
sig_o = fft(double(sig_i),[],1);
sig_o = fi(sig_o,TT,FF);

% ------------------------------------------------------------
% Generate DFT coefficient matrix (matches matlab/wfta.m's wfta7 case
% to within fft()'s own numerical precision - both compute the same
% size-7 DFT; fft(eye(N)) directly gives the NxN DFT matrix since its
% n-th column is the DFT of the n-th unit vector):
% ------------------------------------------------------------

A = fft(eye(Nfft));
Aq = fi(A,TT,'RoundingMethod','Convergent','OverflowAction','Saturate');

fid = fopen('dft7_twiddle.h','w');
fprintf(fid,'//\n');
fprintf(fid,'// Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.\n');
fprintf(fid,'// SPDX-License-Identifier: MIT\n');
fprintf(fid,'//\n');
fprintf(fid,'// Author: Mark Rollins\n');
fprintf(fid,'\n');
fprintf(fid,'#pragma once\n');
fprintf(fid,'#define DFT7_COEFF {\\\n');
for kk = 1 : Nfft
  for nn = 1 : Nfft
    if (kk == Nfft && nn == Nfft) fprintf(fid,'{%d,%d} \\\n',real(Aq.int(kk,nn)),imag(Aq.int(kk,nn)));
    else                          fprintf(fid,'{%d,%d},\\\n',real(Aq.int(kk,nn)),imag(Aq.int(kk,nn)));
    end
  end
end
fprintf(fid,'}\n');
fclose(fid);

% ------------------------------------------------------------
% Save I/O files for simulation
% ------------------------------------------------------------
% Even transforms (columns) go on ss0, odd transforms on ss1. The
% per-stream sample stream is flattened without regard to the 7-sample
% transform boundary (7 is odd, so pairs-of-2-per-line straddle
% transform boundaries - this is resolved downstream purely by sample
% count, matching the original design's convention for odd Nfft).

[~,~,~] = rmdir('data','s');
[~,~,~] = mkdir('data');

fid0_i = fopen('data/sig_i_0.txt','w');
fid1_i = fopen('data/sig_i_1.txt','w');
data_ss0 = reshape(sig_i(:,1:2:end),1,[]);
data_ss1 = reshape(sig_i(:,2:2:end),1,[]);
for ii = 1 : 2 : numel(data_ss0)
  fprintf(fid0_i,'%d %d %d %d\n',...
          real(data_ss0.int(ii+0)),imag(data_ss0.int(ii+0)),...
          real(data_ss0.int(ii+1)),imag(data_ss0.int(ii+1)));
  fprintf(fid1_i,'%d %d %d %d\n',...
          real(data_ss1.int(ii+0)),imag(data_ss1.int(ii+0)),...
          real(data_ss1.int(ii+1)),imag(data_ss1.int(ii+1)));
end
fclose(fid0_i);
fclose(fid1_i);

fid0_o = fopen('data/sig_o_0.txt','w');
fid1_o = fopen('data/sig_o_1.txt','w');
data_ss0 = reshape(sig_o(:,1:2:end),1,[]);
data_ss1 = reshape(sig_o(:,2:2:end),1,[]);
for ii = 1 : 2 : numel(data_ss0)
  fprintf(fid0_o,'%d %d %d %d\n',...
          real(data_ss0.int(ii+0)),imag(data_ss0.int(ii+0)),...
          real(data_ss0.int(ii+1)),imag(data_ss0.int(ii+1)));
  fprintf(fid1_o,'%d %d %d %d\n',...
          real(data_ss1.int(ii+0)),imag(data_ss1.int(ii+0)),...
          real(data_ss1.int(ii+1)),imag(data_ss1.int(ii+1)));
end
fclose(fid0_o);
fclose(fid1_o);
