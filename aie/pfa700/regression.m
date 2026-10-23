%
% Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
% SPDX-License-Identifier: MIT
%
% Author: Mark Rollins

function regression(x86,settings)
   if     (nargin == 0) x86 = 1; settings = [2,8,700];
   elseif (nargin == 1)          settings = [2,8,700];
   elseif (nargin ~= 2) error('Bad voodoo'); end

   close all;
   addpath('../../matlab');
   Nport      = settings(1);
   Ntransform = settings(2);
   Nfft       = settings(3);
   if (x86==0)  base = 'aiesimulator_output';
   else         base = 'x86simulator_output';
   end
   % DFT7/DFT25: even transforms on ss0, odd transforms on ss1.
   act7_o  = zeros(Nfft,Ntransform);
   gld7_o  = zeros(Nfft,Ntransform);
   act25_o = zeros(Nfft,Ntransform);
   gld25_o = zeros(Nfft,Ntransform);
   for pp = 1 : Nport
     % ---------- DFT7 ----------
     tmp = load_complex1(sprintf('%s/data/dft7_o_%d.txt',base,pp-1));
     tmp = reshape(tmp,[],Ntransform/2);
     act7_o(:,pp:2:end) = tmp;
     tmp = load_complex1(sprintf('data/dft7_o_%d.txt',pp-1));
     tmp = reshape(tmp,[],Ntransform/2);
     gld7_o(:,pp:2:end) = tmp;
     % ---------- DFT25 ----------
     tmp = load_complex1(sprintf('%s/data/dft25_o_%d.txt',base,pp-1));
     tmp = reshape(tmp,[],Ntransform/2);
     act25_o(:,pp:2:end) = tmp;
     tmp = load_complex1(sprintf('data/dft25_o_%d.txt',pp-1));
     tmp = reshape(tmp,[],Ntransform/2);
     gld25_o(:,pp:2:end) = tmp;
   end

   % DFT4: split-transform-across-streams (ss0 = X0,X2 - ss1 = X1,X3),
   % one transform per line, Ntransform_dft4 = (N1*N2)*Ntransform frames:
   act4_ss0 = load_complex1(sprintf('%s/data/dft4_o_0.txt',base));
   act4_ss1 = load_complex1(sprintf('%s/data/dft4_o_1.txt',base));
   gld4_ss0 = load_complex1('data/dft4_o_0.txt');
   gld4_ss1 = load_complex1('data/dft4_o_1.txt');
   Ntransform4 = numel(act4_ss0)/2;
   act4_o = zeros(4,Ntransform4);
   gld4_o = zeros(4,Ntransform4);
   act4_o(1,:) = act4_ss0(1:2:end); act4_o(3,:) = act4_ss0(2:2:end);
   act4_o(2,:) = act4_ss1(1:2:end); act4_o(4,:) = act4_ss1(2:2:end);
   gld4_o(1,:) = gld4_ss0(1:2:end); gld4_o(3,:) = gld4_ss0(2:2:end);
   gld4_o(2,:) = gld4_ss1(1:2:end); gld4_o(4,:) = gld4_ss1(2:2:end);

   % Outputs dealt in linear order:
   act7_o  = reshape(act7_o,1,[]);
   gld7_o  = reshape(gld7_o,1,[]);
   act25_o = reshape(act25_o,1,[]);
   gld25_o = reshape(gld25_o,1,[]);
   act4_o  = reshape(act4_o,1,[]);
   gld4_o  = reshape(gld4_o,1,[]);
   err7  = act7_o  - gld7_o;
   err25 = act25_o - gld25_o;
   err4  = act4_o  - gld4_o;

   if (1)
     subplot(3,3,1); plot(real(gld7_o),'b.-'); hold on; plot(real(act7_o),'r.--'); hold off; ylabel('real');
     subplot(3,3,2); plot(imag(gld7_o),'b.-'); hold on; plot(imag(act7_o),'r.--'); hold off; ylabel('imag');
     subplot(3,3,3); plot(real(err7),'k.-'); hold on; plot(imag(err7),'m.--'); hold off; ylabel('imag');
     subplot(3,3,4); plot(real(gld25_o),'b.-'); hold on; plot(real(act25_o),'r.--'); hold off; ylabel('real');
     subplot(3,3,5); plot(imag(gld25_o),'b.-'); hold on; plot(imag(act25_o),'r.--'); hold off; ylabel('imag');
     subplot(3,3,6); plot(real(err25),'k.-'); hold on; plot(imag(err25),'m.--'); hold off; ylabel('imag');
     subplot(3,3,7); plot(real(gld4_o),'b.-'); hold on; plot(real(act4_o),'r.--'); hold off; ylabel('real');
     subplot(3,3,8); plot(imag(gld4_o),'b.-'); hold on; plot(imag(act4_o),'r.--'); hold off; ylabel('imag');
     subplot(3,3,9); plot(real(err4),'k.-'); hold on; plot(imag(err4),'m.--'); hold off; ylabel('imag');
   end
   max_err = max([abs(real(err7)),abs(imag(err7)),abs(real(err25)),abs(imag(err25)),abs(real(err4)),abs(imag(err4))]);
   fprintf(1,'Max Err: %g\n',max_err);
   if ( max_err <= 33 )
     fprintf(1,'--- PASSED ---\n');
   else
     fprintf(1,'*** FAILED ***\n');
   end
end

% Reads "re im" (one complex sample per line) files - the plio_32_bits
% convention used at the pfa700 top-level graph boundary:
function [data] = load_complex1( fname )
   raw = load_aiesim(fname,'int',0);
   data = complex(raw(1:2:end),raw(2:2:end));
end
