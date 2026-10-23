%
% Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
% SPDX-License-Identifier: MIT
%
% Author: Mark Rollins

clear all;
close all;
addpath('../../matlab');

% ------------------------------------------------------------
% Buffer index convention: dft4's OUTPUT natural flat/cube order
% (0-indexed j3, the (N1,N2,N3) cube flattened column-major).
% ------------------------------------------------------------

N1 = 7;
N2 = 25;
N3 = 4;
N = N1*N2*N3;
[P_i,P_o] = compute_perm_3d(N1,N2,N3);

% ---- WRITE side: dft4's split-transform-across-streams convention ----
% (identity on the buffer-index space; dft4's per-group ss0=(x0,x2),
% ss1=(x1,x3) convention reduces to simple even/odd indices when
% flattened across all N1*N2 groups - see aie/pfa700/gen_vectors.m):
idx = 0:N-1;
WR_ADDR0 = idx(1:2:end);
WR_ADDR1 = idx(2:2:end);

% ---- READ side: dma_snk's simple round-robin convention ----
% Final output position m needs buffer[inv_P_o(m)], since
% sig_o(1+P_o) = data_o means sig_o[P_o(j3)] = buffer[j3], i.e. for
% output position m = P_o(j3), we read buffer[j3] = buffer[inv_P_o(m)]:
inv_P_o = zeros(1,N);
inv_P_o(1+P_o) = 0:N-1;
m = 0:N-1;
tmp = reshape(inv_P_o(1+m),2,[]);
RD_ADDR0 = tmp(1,:);
RD_ADDR1 = tmp(2,:);

% ------------------------------------------------------------
% Write Output Header
% ------------------------------------------------------------

fid = fopen('pfa700_permute_o_luts.h','w');
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
