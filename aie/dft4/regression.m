%
% Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
% SPDX-License-Identifier: MIT
%
% Author: Mark Rollins

function regression(x86,settings)
   if     (nargin == 0) x86 = 1; settings = [7*25,4];
   elseif (nargin == 1)          settings = [7*25,4];
   elseif (nargin ~= 2) error('Bad voodoo'); end

   close all;
   addpath('../../matlab');
   Ntransform = settings(1);
   Nfft       = settings(2);
   if (x86==0)  base = 'aiesimulator_output';
   else         base = 'x86simulator_output';
   end

   % ss0 carries (X0,X2) per transform, ss1 carries (X1,X3) per transform
   act_ss0 = load_aiesim(sprintf('%s/data/sig_o_0.txt',base),'int',1);
   act_ss1 = load_aiesim(sprintf('%s/data/sig_o_1.txt',base),'int',1);
   gld_ss0 = load_aiesim('data/sig_o_0.txt','int',1);
   gld_ss1 = load_aiesim('data/sig_o_1.txt','int',1);

   act_o = zeros(Nfft,Ntransform);
   gld_o = zeros(Nfft,Ntransform);
   act_o(1,:) = act_ss0(1:2:end); act_o(3,:) = act_ss0(2:2:end);
   act_o(2,:) = act_ss1(1:2:end); act_o(4,:) = act_ss1(2:2:end);
   gld_o(1,:) = gld_ss0(1:2:end); gld_o(3,:) = gld_ss0(2:2:end);
   gld_o(2,:) = gld_ss1(1:2:end); gld_o(4,:) = gld_ss1(2:2:end);

   act_o = reshape(act_o,1,[]);
   gld_o = reshape(gld_o,1,[]);
   err = act_o - gld_o;

   if (1)
     subplot(3,1,1); plot(real(gld_o),'b.-'); hold on; plot(real(act_o),'r.--'); hold off; ylabel('real');
     subplot(3,1,2); plot(imag(gld_o),'b.-'); hold on; plot(imag(act_o),'r.--'); hold off; ylabel('imag');
     subplot(3,1,3); plot(real(err),'k.-'); hold on; plot(imag(err),'m.-'); hold off; ylabel('abs(err)');
   end
   max_err = max([abs(real(err)),abs(imag(err))]);
   fprintf(1,'Max Err: %g\n',max_err);
   if ( max_err <= 33 )
     fprintf(1,'--- PASSED ---\n');
   else
     fprintf(1,'*** FAILED ***\n');
   end
end
