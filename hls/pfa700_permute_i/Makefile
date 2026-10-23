#
# Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
# SPDX-License-Identifier: MIT
#
# Author: Mark Rollins

DO_COSIM        := false
DO_OOCPAR       := false
ECHO            := @echo

KERNEL          := pfa700_permute_i_wrapper
KERNEL_SRC      := pfa700_permute_i.cpp
XO              := ${KERNEL}.xo

.PHONY: clean all help csim

help::
	$(ECHO) "Makefile Usage:"
	$(ECHO) "  make all"
	$(ECHO) "      Command to generate the kernel used in this design"
	$(ECHO) ""
	$(ECHO) "  make csim"
	$(ECHO) "      Command to run C-simulation against the testbench"
	$(ECHO) ""
	$(ECHO) "  make clean"
	$(ECHO) "      Command to remove all the generated files."

all: ${XO}

${XO}: ${KERNEL_SRC}
	v++ --config hls.cfg --compile --mode hls --work_dir ${KERNEL}
ifeq (${DO_COSIM},true)
	vitis-run --mode hls --config hls.cfg --csim --work_dir ${KERNEL}
	vitis-run --mode hls --config hls.cfg --cosim --work_dir ${KERNEL}
endif
ifeq (${DO_OOCPAR},true)
	vitis-run --mode hls --config hls.cfg --impl --work_dir ${KERNEL}
endif

csim:
	vitis-run --mode hls --config hls.cfg --csim --work_dir ${KERNEL}

gen_permute_tables:
	matlab -batch "gen_permute_tables"

clean:
	rm -f $(XO)* *.log vitis-comp.json
	rm -rf .Xil _x/ .crashReporter*
	rm -rf ${KERNEL}
