%
% Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
% SPDX-License-Identifier: MIT
%
% Author: Mark Rollins

clear all;
close all;
addpath('../../matlab');

% ------------------------------------------------------------
% Buffer index convention: INPUT natural flat order (0-indexed, matches
% dma_src's simple round-robin stream convention and the pre-permutation
% sample order used throughout aie/pfa700/gen_vectors.m).
% ------------------------------------------------------------

N1 = 7;
N2 = 25;
N3 = 4;
N = N1*N2*N3;
[P_i,P_o] = compute_perm_3d(N1,N2,N3);

% ---- WRITE side: dma_src's round-robin convention (0-indexed) ----
% k even -> ss0, k odd -> ss1 (matches "data_i = reshape(sig_i,2,[])"):
idx = 0:N-1;
tmp = reshape(idx,2,[]);
WR_ADDR0 = tmp(1,:);
WR_ADDR1 = tmp(2,:);

% ---- READ side: dft7w's independent-transforms-per-stream convention ----
% Output flat position j (0-indexed) needs buffer[P_i(j)] (matches
% fft_pfa_3d.m: "data = reshape(sig_i(1+P_i),N1,N2,N3)"). Split by N1
% columns exactly like aie/dft7w/gen_vectors.m does:
src = P_i; % 0-indexed source buffer address for each output flat position j
tmp = reshape(src,N1,[]);
RD_ADDR0 = reshape(tmp(:,1:2:end),1,[]);
RD_ADDR1 = reshape(tmp(:,2:2:end),1,[]);

% ------------------------------------------------------------
% Write Output Header
% ------------------------------------------------------------

fid = fopen('pfa700_permute_i_luts.h','w');
fprintf(fid,'//\n');
fprintf(fid,'// Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.\n');
fprintf(fid,'// SPDX-License-Identifier: MIT\n');
fprintf(fid,'//\n');
fprintf(fid,'// Author: Mark Rollins\n');
fprintf(fid,'\n');
write_lut(fid,'WR_ADDR0',WR_ADDR0);
write_lut(fid,'WR_ADDR1',WR_ADDR1);
write_lut(fid,'RD_ADDR0',RD_ADDR0);
write_lut(fid,'RD_ADDR1',RD_ADDR1);
fclose(fid);

function write_lut( fid, name, addr )
   fprintf(fid,'#define %s {\\\n',name);
   for ii = 1 : numel(addr)
     if ( ii == numel(addr) ) fprintf(fid,'%4d \\\n',addr(ii));
     else                     fprintf(fid,'%4d,\\\n',addr(ii));
     end
   end
   fprintf(fid,'}\n');
end
