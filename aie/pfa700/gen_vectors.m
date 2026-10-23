%
% Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
% SPDX-License-Identifier: MIT
%
% Author: Mark Rollins

clear all;
close all;

rng(1);

% ------------------------------------------------------------
% Generate Model
% ------------------------------------------------------------

Nports = 2;
Nfft = 700;
N1 = 7;
N2 = 25;
N3 = 4;
Ntransform = 8;
backoff_dB = -15;                       % To achieve (-1,+1) range on AWGN signal

% Downshift (to ensure no clipping occurs in FFT):
TT = numerictype(1,16,15);
FF = fimath('RoundingMethod','Round','OverflowAction','Saturate');

% Input signal:
taps = firgr('minevn',[0,0.3,0.4,1.0],[1,1,0,0],[1e-4,1e-4]);
fir_i = sqrt(0.5)*10^(0.05*backoff_dB)*complex(randn(1,Nfft*Ntransform),randn(1,Nfft*Ntransform));
fir_o = filter(taps,1,fir_i);
scale = 1/32;
sig_i = fi(reshape(scale*fir_o,Nfft,Ntransform),TT,FF);

% Apply FFT-700 using PFA architecture:
addpath('../../matlab');
[P_i,P_o] = compute_perm_3d(N1,N2,N3);
dft7_i  = fi(complex(zeros(N1,N2,N3,Ntransform),zeros(N1,N2,N3,Ntransform)),TT,FF);
dft7_o  = fi(complex(zeros(N1,N2,N3,Ntransform),zeros(N1,N2,N3,Ntransform)),TT,FF);
dft25_o = fi(complex(zeros(N1,N2,N3,Ntransform),zeros(N1,N2,N3,Ntransform)),TT,FF);
dft4_o  = fi(complex(zeros(N1,N2,N3,Ntransform),zeros(N1,N2,N3,Ntransform)),TT,FF);
sig_o   = fi(complex(zeros(Nfft,Ntransform),zeros(Nfft,Ntransform)),TT,FF);
for tt = 1 : Ntransform
  % DFT-7 (N1 dimension):
  dft7_i(:,:,:,tt) = reshape(sig_i(1+P_i,tt),N1,N2,N3);
  for cc = 1 : N2
    for dd = 1 : N3
      dft7_o(:,cc,dd,tt) = fi(fft(double(dft7_i(:,cc,dd,tt))),TT,FF);
    end
  end
  % DFT-25 (N2 dimension):
  for rr = 1 : N1
    for dd = 1 : N3
      dft25_o(rr,:,dd,tt) = fi(fft(double(dft7_o(rr,:,dd,tt))),TT,FF);
    end
  end
  % DFT-4 (N3 dimension):
  for rr = 1 : N1
    for cc = 1 : N2
      dft4_o(rr,cc,:,tt) = fi(fft(double(dft25_o(rr,cc,:,tt))),TT,FF);
    end
  end
  sig_o(1+P_o,tt) = reshape(dft4_o(:,:,:,tt),1,[]);
end

% ------------------------------------------------------------
% Save I/O files for Top Level AIE-Only Simulation
% ------------------------------------------------------------

[~,~,~] = rmdir('data','s');
[~,~,~] = mkdir('data');

% -------------------- DFT-7 I/O Files --------------------
% Independent-transforms-per-stream convention (matches aie/dft7w):
% even transforms (columns) on ss0, odd on ss1; flattened without
% regard to the 7-sample transform boundary (7 is odd).

fi0 = fopen('data/dft7_i_0.txt','w');
fi1 = fopen('data/dft7_i_1.txt','w');
fo0 = fopen('data/dft7_o_0.txt','w');
fo1 = fopen('data/dft7_o_1.txt','w');
for tt = 1 : Ntransform
  data_i = reshape(dft7_i(:,:,:,tt),N1,[]);
  data_o = reshape(dft7_o(:,:,:,tt),N1,[]);
  ss0_i = reshape(data_i(:,1:2:end),1,[]);
  ss1_i = reshape(data_i(:,2:2:end),1,[]);
  ss0_o = reshape(data_o(:,1:2:end),1,[]);
  ss1_o = reshape(data_o(:,2:2:end),1,[]);
  for ii = 1 : numel(ss0_i)
    fprintf(fi0,'%d %d\n',real(ss0_i(ii)).int,imag(ss0_i(ii)).int);
    fprintf(fi1,'%d %d\n',real(ss1_i(ii)).int,imag(ss1_i(ii)).int);
    fprintf(fo0,'%d %d\n',real(ss0_o(ii)).int,imag(ss0_o(ii)).int);
    fprintf(fo1,'%d %d\n',real(ss1_o(ii)).int,imag(ss1_o(ii)).int);
  end
end
fclose(fi0);
fclose(fi1);
fclose(fo0);
fclose(fo1);

% -------------------- DFT-25 I/O Files --------------------
% Same independent-transforms-per-stream convention (matches aie/dft25w).

fi0 = fopen('data/dft25_i_0.txt','w');
fi1 = fopen('data/dft25_i_1.txt','w');
fo0 = fopen('data/dft25_o_0.txt','w');
fo1 = fopen('data/dft25_o_1.txt','w');
addr_N2 = compute_addr_3d(N1,N2,N3,2);
for tt = 1 : Ntransform
  data_i = reshape(dft7_o(:,:,:,tt),1,[]);  data_i = data_i(1+addr_N2); data_i = reshape(data_i,N2,[]);
  data_o = reshape(dft25_o(:,:,:,tt),1,[]); data_o = data_o(1+addr_N2); data_o = reshape(data_o,N2,[]);
  ss0_i = reshape(data_i(:,1:2:end),1,[]);
  ss1_i = reshape(data_i(:,2:2:end),1,[]);
  ss0_o = reshape(data_o(:,1:2:end),1,[]);
  ss1_o = reshape(data_o(:,2:2:end),1,[]);
  for ii = 1 : numel(ss0_i)
    fprintf(fi0,'%d %d\n',real(ss0_i(ii)).int,imag(ss0_i(ii)).int);
    fprintf(fi1,'%d %d\n',real(ss1_i(ii)).int,imag(ss1_i(ii)).int);
    fprintf(fo0,'%d %d\n',real(ss0_o(ii)).int,imag(ss0_o(ii)).int);
    fprintf(fo1,'%d %d\n',real(ss1_o(ii)).int,imag(ss1_o(ii)).int);
  end
end
fclose(fi0);
fclose(fi1);
fclose(fo0);
fclose(fo1);

% -------------------- DFT-4 I/O Files --------------------
% Split-transform-across-streams convention (matches aie/dft4): NFFT
% (= N1*N2 = 175) is odd, so the 2 streams instead each carry half of
% every transform - ss0 gets samples (x0,x2), ss1 gets (x1,x3), one
% transform per line.

fi0 = fopen('data/dft4_i_0.txt','w');
fi1 = fopen('data/dft4_i_1.txt','w');
fo0 = fopen('data/dft4_o_0.txt','w');
fo1 = fopen('data/dft4_o_1.txt','w');
addr_N3 = compute_addr_3d(N1,N2,N3,3);
for tt = 1 : Ntransform
  data_i = reshape(dft25_o(:,:,:,tt),1,[]); data_i = data_i(1+addr_N3); data_i = reshape(data_i,4,[]);
  data_o = reshape(dft4_o(:,:,:,tt),1,[]);  data_o = data_o(1+addr_N3); data_o = reshape(data_o,4,[]);
  ss0_i = data_i([1,3],:);
  ss1_i = data_i([2,4],:);
  ss0_o = data_o([1,3],:);
  ss1_o = data_o([2,4],:);
  for cc = 1 : size(data_i,2)
    fprintf(fi0,'%d %d\n',real(ss0_i(1,cc)).int,imag(ss0_i(1,cc)).int);
    fprintf(fi0,'%d %d\n',real(ss0_i(2,cc)).int,imag(ss0_i(2,cc)).int);
    fprintf(fi1,'%d %d\n',real(ss1_i(1,cc)).int,imag(ss1_i(1,cc)).int);
    fprintf(fi1,'%d %d\n',real(ss1_i(2,cc)).int,imag(ss1_i(2,cc)).int);
    fprintf(fo0,'%d %d\n',real(ss0_o(1,cc)).int,imag(ss0_o(1,cc)).int);
    fprintf(fo0,'%d %d\n',real(ss0_o(2,cc)).int,imag(ss0_o(2,cc)).int);
    fprintf(fo1,'%d %d\n',real(ss1_o(1,cc)).int,imag(ss1_o(1,cc)).int);
    fprintf(fo1,'%d %d\n',real(ss1_o(2,cc)).int,imag(ss1_o(2,cc)).int);
  end
end
fclose(fi0);
fclose(fi1);
fclose(fo0);
fclose(fo1);

% ------------------------------------------------------------
% Save I/O files for Top Level HW Emulation
% ------------------------------------------------------------

% Input samples to DDR in linear order:
fid_i = fopen('data/sig_i.txt','w');
sig_i_lin = reshape(sig_i,1,[]);
for ii = 1 : numel(sig_i_lin)
  fprintf(fid_i,'%d %d \n',real(sig_i_lin.int(ii)),imag(sig_i_lin.int(ii+0)));
end
fclose(fid_i);

% Output samples to DDR in linear order:
fid_o = fopen('data/sig_o.txt','w');
sig_o_lin = reshape(sig_o,1,[]);
for ii = 1 : numel(sig_o_lin)
  fprintf(fid_o,'%d %d \n',real(sig_o_lin.int(ii+0)),imag(sig_o_lin.int(ii+0)));
end
fclose(fid_o);
