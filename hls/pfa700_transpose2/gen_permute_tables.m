%
% Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
% SPDX-License-Identifier: MIT
%
% Author: Mark Rollins

clear all;
close all;
addpath('../../matlab');

% ------------------------------------------------------------
% Buffer index convention: dft25w's OUTPUT natural flat/cube order
% (0-indexed j2, the (N1,N2,N3) cube flattened column-major).
% ------------------------------------------------------------

N1 = 7;
N2 = 25;
N3 = 4;
N = N1*N2*N3;
addr_N3 = compute_addr_3d(N1,N2,N3,3);

% ---- WRITE side: dft25w's independent-transforms-per-stream convention ----
% (identity on the buffer-index space, split by N2 columns):
idx = 0:N-1;
tmp = reshape(idx,N2,[]);
WR_ADDR0 = reshape(tmp(:,1:2:end),1,[]);
WR_ADDR1 = reshape(tmp(:,2:2:end),1,[]);

% ---- READ side: dft4's split-transform-across-streams convention ----
% Output flat position j3 needs buffer[addr_N3(j3)] (matches
% aie/pfa700/gen_vectors.m's dft4 section). dft4's convention splits
% each 4-sample group as ss0=(x0,x2), ss1=(x1,x3); flattened across all
% N1*N2 groups this reduces to simple even/odd indices (see
% aie/pfa700/gen_vectors.m comment for the derivation):
src = addr_N3; % 0-indexed source buffer address for each output flat position j3
RD_ADDR0 = src(1:2:end);
RD_ADDR1 = src(2:2:end);

% ------------------------------------------------------------
% Write Output Header
% ------------------------------------------------------------

fid = fopen('pfa700_transpose2_luts.h','w');
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
