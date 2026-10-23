%
% Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
% SPDX-License-Identifier: MIT
%
% Author: Mark Rollins

clear all;
close all;
addpath('../../matlab');

% ------------------------------------------------------------
% Buffer index convention: dft7w's OUTPUT natural flat/cube order
% (0-indexed j, the (N1,N2,N3) cube flattened column-major - same
% convention aie/dft7w/gen_vectors.m and aie/pfa700/gen_vectors.m use).
% ------------------------------------------------------------

N1 = 7;
N2 = 25;
N3 = 4;
N = N1*N2*N3;
addr_N2 = compute_addr_3d(N1,N2,N3,2);

% ---- WRITE side: dft7w's independent-transforms-per-stream convention ----
% (identity on the buffer-index space, split by N1 columns like
% aie/dft7w/gen_vectors.m does):
idx = 0:N-1;
tmp = reshape(idx,N1,[]);
WR_ADDR0 = reshape(tmp(:,1:2:end),1,[]);
WR_ADDR1 = reshape(tmp(:,2:2:end),1,[]);

% ---- READ side: dft25w's independent-transforms-per-stream convention ----
% Output flat position j2 needs buffer[addr_N2(j2)] (matches
% aie/pfa700/gen_vectors.m's dft25 section). Split by N2 columns:
src = addr_N2; % 0-indexed source buffer address for each output flat position j2
tmp = reshape(src,N2,[]);
RD_ADDR0 = reshape(tmp(:,1:2:end),1,[]);
RD_ADDR1 = reshape(tmp(:,2:2:end),1,[]);

% ------------------------------------------------------------
% Write Output Header
% ------------------------------------------------------------

fid = fopen('pfa700_transpose1_luts.h','w');
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
