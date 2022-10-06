# Ensure that there are no spaces in path to avoid any undesired behavior
ifneq ($(word 2,$(realpath $(lastword $(MAKEFILE_LIST)))),)
$(error "ERROR: folder path contains spaces")
endif

MAKEFILEDIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))
MAKEFILEDIRNAME := $(basename $(MAKEFILEDIR))

GCD_SV_SOURCES := gcd.sv testbench.sv
SIMDIR := $(MAKEFILEDIR)_sim/
SYNTHDIR := $(MAKEFILEDIR)_synth/

GCD_SV := gcd

IVERILOGFLAGS ?= -Wall -g2012 -Wno-sensitivity-entire-array -I$(MAKEFILEDIR)

GTKWAVESAVE   ?= $(MAKEFILEDIR)$(GCD_SV).gtkw

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

$(SIMDIR)gcd.vvp: $(GCD_SV_SOURCES)
	@mkdir -p $(SIMDIR)
	$(IVERILOG) $(IVERILOGFLAGS) -o $@ $(GCD_SV_SOURCES)

#help
#help	build	builds the gcd module
build: $(SIMDIR)gcd.vvp ;

#help
#help	test	runs the gcd in the testbench
test: $(SIMDIR)gcd.vvp
	$(SIMDIR)gcd.vvp

#help
#help	view	runs the gcd in gtkwave
view: $(SIMDIR)gcd.vvp
	$(SIMDIR)gcd.vvp
	$(GTKWAVE) --save $(GCD_SV).gtkw $(SIMDIR)gcd.vcd

#help
#help 	synth	synthesizes the design with yosys
synth:
	@mkdir -p $(SYNTHDIR)
	sv2v -E logic $(GCD_SV).sv -w $(SYNTHDIR)/gcd.v
	$(YOSYS) -T synth.ys
