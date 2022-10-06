#!/usr/bin/env python3
import argparse
from asmlib import Assembler, Parsing, Riscv, Simulation
import os
import sys
import math
import operator
from typing import List, Tuple, Optional


class GcdAccelerator(Simulation.MemorySubsystem, Simulation.PeripheralDevice):
    def __init__(self, simulator: Optional[Simulation.Simulator],
                 address: int):
        super().__init__(simulator)
        self._address = address
        self._name = "GCD"
        self._a_reg = 0
        self._b_reg = 0
        self._result_reg = 0
        self._delay = -1
        self._irq_enabled = False
        self._valid = False
        self._old_valid = False

    def bounds(self) -> Tuple[int, int]:
        return (self._address, self._address+4*4-1)

    def read(self, addr: int, size: int = 1) -> bytes:
        assert size == 4
        addr -= self._address
        if addr == 0:
            return (self._irq_enabled << 3 | self._valid << 2 | (int(self._delay > 0) << 1 | 0) << 0).to_bytes(4, 'little')
        elif addr == 4:
            return self._data_reg.to_bytes(4, 'little')
        elif addr == 8:
            return self._data_reg.to_bytes(4, 'little')
        elif addr == 12:
            return self._result_reg.to_bytes(4, 'little')
        raise Simulation.SimulationError("Reading invalid address.", self.simulator)

    def write(self, addr: int, data: List[int]):
        assert len(data) == 4
        if self._delay > 0:
            raise Simulation.SimulationError("Writing to busy accelerator.", self.simulator)
        addr -= self._address
        if addr == 0:
            value = int.from_bytes(data, 'little')
            if value & 0x1:
                self._delay = 30
                self._valid = False
            if value & 0x8:
                self._irq_enabled = True
            else:
                self._irq_enabled = False
        elif addr == 4:
            self._a_reg = int.from_bytes(data, 'little')
        elif addr == 8:
            self._b_reg = int.from_bytes(data, 'little')
        else:
            raise Simulation.SimulationError("Writing to invalid address.", self.simulator)

    def tick(self, simulator: Optional[Simulation.Simulator]):
        if self._delay == 0:
            self._result_reg = math.gcd(self._a_reg, self._b_reg)
            self._valid = True

        if self._delay >= 0:
            self._delay -= 1

    def raise_interrupt(self, simulator: Optional[Simulation.Simulator]):
        raise_irq = False
        if(self._valid and not self._old_valid):
            raise_irq = True
        self._old_valid = self._valid
        return raise_irq and self._irq_enabled


def update_simulator_devices(arbiter: Simulation.MemoryArbiter,
                             sim: Simulation.Simulator,
                             args: argparse.Namespace):
    gcd_module = GcdAccelerator(sim, 0x7D0)
    arbiter.devices.insert(0, gcd_module)
    sim.add_peripheral(gcd_module, True)
