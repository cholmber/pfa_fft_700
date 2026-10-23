#
# Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
# SPDX-License-Identifier: MIT
#
# Author: Mark Rollins

AIE_SIM_ONLY      := false

SIM_FIFO          := false

MY_APP            := pfa700_app

DFT7_SOURCE       := dft7_graph.h dft7_compute.h dft7_compute.cpp dft7_twiddle.h

DFT25_SOURCE      := dft25_graph.h dft25_compute.h dft25_compute.cpp dft25_twiddle.h

DFT4_SOURCE       := dft4_graph.h dft4_compute.h dft4_compute.cpp

MY_SOURCES        := ${MY_APP}.cpp pfa700_graph.h ${DFT7_SOURCE:%=../dft7w/%} ${DFT25_SOURCE:%=../dft25w/%} \
                     ${DFT4_SOURCE:%=../dft4/%}

PART_USE          := xcvc1902-vsva2197-2MP-e-S

PL_FREQ           := --aie.pl-freq=625
OTHER_OPTS        := --aie.kernel-linting --aie.Xchess="-std=c++17" --aie.xlopt=1
MAP_OPT           := --aie.Xmapper=BufferOptLevel9

CHECK_FIFO        := --aie.evaluate-fifo-depth --aie.Xrouter=disablePathBalancing

DSPLIB_OPTS 	  := --include=${DSPLIB_ROOT}/L2/include/aie \
		     --include=${DSPLIB_ROOT}/L1/include/aie \
		     --include=${DSPLIB_ROOT}/L1/src/aie \
		     --include=../dft7w --include=../dft25w --include=../dft4

AIE_OUTPUT := libadf.a

AIE_FLAGS :=	${OTHER_OPTS} ${DSPLIB_OPTS} ${MAP_OPT} ${PL_FREQ} --part=${PART_USE} ${MY_APP}.cpp --aie.output-archive=${AIE_OUTPUT}

ifeq (${SIM_FIFO}, true)
	AIE_FLAGS := ${AIE_FLAGS} ${CHECK_FIFO}
endif
ifeq (${AIE_SIM_ONLY},true)
	AIE_FLAGS := ${AIE_FLAGS} --Xpreproc="-DAIE_SIM_ONLY"
endif

.PHONY: help clean x86com x86sim sim profile throughput

help::
	@echo "Makefile Usage:"
	@echo "  make all"
	@echo "      Command to generate everything for this design"
	@echo ""
	@echo "  make compile"
	@echo "      Run AIE compiler and build the design"
	@echo ""
	@echo "  make profile"
	@echo "      Run AIE simulator with profiling and generate outputs suitable for Vitis Analyzer"
	@echo ""
	@echo "  make analyze"
	@echo "      Run Vitis Analyzer to inspect results"
	@echo ""
	@echo "  make clean"
	@echo "      Command to remove all the generated files."

all:		${AIE_OUTPUT}

x86all: 	clean x86com x86sim

gen_vectors:
	matlab -batch "gen_vectors"

${AIE_OUTPUT}:	${MY_SOURCES}
	v++ --compile --mode aie --target=hw ${AIE_FLAGS} 2>&1| tee log

x86com:
	v++ --compile --mode aie --target=x86sim ${AIE_FLAGS} 2>&1| tee log

sim:
	aiesimulator 2>&1| tee -a log

x86sim:
	x86simulator 2>&1| tee -a log

profile:
	aiesimulator --profile 2>&1| tee -a log

trace:
	aiesimulator --online -wdb -ctf 2>&1| tee -a log

regression:
	matlab -batch regression

analyze:
	vitis_analyzer aiesimulator_output/default.aierun_summary

clean:
	rm -rf .Xil Work libadf.a
	rm -rf aiesimulator_output* aiesimulator*.log
	rm -rf x86simulator_output*
	rm -rf log log*
	rm -rf *.xpe *.elf *.db *.soln Map_* xnw* *.lp *.log .xil .Xil *.lp *.db *.log *.exe *.vcd *.json
	rm -rf vitis_analyzer* pl_sample_counts* pl_sample_count_*
	rm -rf temp ISS_RPC_SERVER_PORT .crashReporter .AIE_SIM_CMD_LINE_OPTIONS
	rm -rf system*.* trdata.aiesim
