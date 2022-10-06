# Ensure that there are no spaces in path to avoid any undesired behavior
ifneq ($(word 2,$(realpath $(lastword $(MAKEFILE_LIST)))),)
$(error "ERROR: folder path contains spaces")
endif

MAKEFILEDIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))
MAKEFILEDIRNAME := $(basename $(MAKEFILEDIR))

MULTIPLIER_SV_SOURCES := multiplier.sv testbench.sv
SIMDIR := $(MAKEFILEDIR)_sim/
SYNTHDIR := $(MAKEFILEDIR)_synth/

MULTIPLIER_SV := multiplier

IVERILOGFLAGS ?= -Wall -g2012 -Wno-sensitivity-entire-array -I$(MAKEFILEDIR)

GTKWAVESAVE   ?= $(MAKEFILEDIR)$(MULTIPLIER_SV).gtkw

ifeq (, $(shell which iverilog))
  IVERILOG      := iverilog-11 -g2012
else
  IVERILOG      := iverilog -g2012
endif

GTKWAVE  ?= gtkwave
YOSYS    ?= yosys

#help
#helpThe main targets of this Makefile are:
#help	all	generates the verilog simulator
.PHONY: all
all: build

#help	clean	clean up temporary files
.PHONY: clean
clean:
	rm -rf $(SIMDIR)
	rm -rf $(SYNTHDIR)

#help	help	displays this help
.PHONY: help
help:
	@cat ${MAKEFILE_LIST} | sed -n 's/^#help//p'

$(SIMDIR)multiplier.vvp: $(MULTIPLIER_SV_SOURCES)
	@mkdir -p $(SIMDIR)
	$(IVERILOG) $(IVERILOGFLAGS) -o $@ $(MULTIPLIER_SV_SOURCES)

#help
#help	build	builds the multiplier module
build: $(SIMDIR)multiplier.vvp ;

#help
#help	test	runs the multiplier in the testbench
test: $(SIMDIR)multiplier.vvp
	$(SIMDIR)multiplier.vvp

#help
#help	view	runs the multiplier in gtkwave
view: $(SIMDIR)multiplier.vvp
	$(SIMDIR)multiplier.vvp
	$(GTKWAVE) --save $(MULTIPLIER_SV).gtkw $(SIMDIR)multiplier.vcd

#help
#help 	synth	synthesizes the design with yosys
synth:
	@mkdir -p $(SYNTHDIR)
	sv2v -E logic $(MULTIPLIER_SV).sv -w $(SYNTHDIR)/multiplier.v
	$(YOSYS) -T synth.ys
