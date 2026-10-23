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
% NFFT (= N1*N2 = 7*25 = 175, the other two PFA-700 factors) is odd, so
% unlike dft7/dft25 the 2 streams cannot each carry independent whole
% transforms. Instead (matching the original design's convention for its
% power-of-2 kernel) a single transform is split across both streams:
% ss0 carries the even-indexed samples (x0,x2), ss1 the odd-indexed
% samples (x1,x3); same convention on output (ss0: X0,X2 - ss1: X1,X3).

Nfft = 4;
Ntransform = 7*25;
backoff_dB = -15;                       % To achieve (-1,+1) range on AWGN signal

TT = numerictype(1,16,15);
FF = fimath('RoundingMethod','Round','OverflowAction','Saturate');

% Input signal:
taps = firgr('minevn',[0,0.3,0.4,1.0],[1,1,0,0],[1e-4,1e-4]);
fir_i = sqrt(0.5)*10^(0.05*backoff_dB)*complex(randn(1,Nfft*Ntransform),randn(1,Nfft*Ntransform));
fir_o = filter(taps,1,fir_i);
scale = 1/32;
sig_i = fi(reshape(scale*fir_o,Nfft,Ntransform),TT,FF);

% Output (plain DFT - dft4 is a radix-4 butterfly with only +-1/+-j
% factors, so no coefficient quantization to worry about):
sig_o = fft(double(sig_i),[],1);
sig_o = fi(sig_o,TT,FF);

% ------------------------------------------------------------
% Save I/O files for simulation
% ------------------------------------------------------------

[~,~,~] = rmdir('data','s');
[~,~,~] = mkdir('data');

% Input: ss0 = (x0,x2) per transform, ss1 = (x1,x3) per transform
ss0_i = sig_i([1,3],:);
ss1_i = sig_i([2,4],:);
fid0_i = fopen('data/sig_i_0.txt','w');
fid1_i = fopen('data/sig_i_1.txt','w');
for tt = 1 : Ntransform
  fprintf(fid0_i,'%d %d %d %d\n',...
          real(ss0_i.int(1,tt)),imag(ss0_i.int(1,tt)),...
          real(ss0_i.int(2,tt)),imag(ss0_i.int(2,tt)));
  fprintf(fid1_i,'%d %d %d %d\n',...
          real(ss1_i.int(1,tt)),imag(ss1_i.int(1,tt)),...
          real(ss1_i.int(2,tt)),imag(ss1_i.int(2,tt)));
end
fclose(fid0_i);
fclose(fid1_i);

% Output: ss0 = (X0,X2) per transform, ss1 = (X1,X3) per transform
ss0_o = sig_o([1,3],:);
ss1_o = sig_o([2,4],:);
fid0_o = fopen('data/sig_o_0.txt','w');
fid1_o = fopen('data/sig_o_1.txt','w');
for tt = 1 : Ntransform
  fprintf(fid0_o,'%d %d %d %d\n',...
          real(ss0_o.int(1,tt)),imag(ss0_o.int(1,tt)),...
          real(ss0_o.int(2,tt)),imag(ss0_o.int(2,tt)));
  fprintf(fid1_o,'%d %d %d %d\n',...
          real(ss1_o.int(1,tt)),imag(ss1_o.int(1,tt)),...
          real(ss1_o.int(2,tt)),imag(ss1_o.int(2,tt)));
end
fclose(fid0_o);
fclose(fid1_o);
