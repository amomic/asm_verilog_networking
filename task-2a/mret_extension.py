#!/usr/bin/env python3
from asmlib import RawType, Assembler, Simulation, impl_mret
from typing import List, Union
import argparse

# This extension is necessary to fix a bug in asmlib with the encoding of the MRET instruction
def update_assembler_isa(isa: List[Union[Assembler.InstDescription, Simulation.InstDescription]],
                         cli_args: argparse.Namespace):
    for insn in isa:
        if insn.mnemonic == "MRET":
            isa.remove(insn)
            break
    isa.append(RawType("MRET", 0x30200073, impl_mret))


update_simulator_isa = update_assembler_isa

