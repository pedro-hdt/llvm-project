//===-- TinyRISCV.h - Top-level interface for TinyRISCV -----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the entry points for global functions defined in the LLVM
// RISC-V back-end.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_TinyRISCV_TinyRISCV_H
#define LLVM_LIB_TARGET_TinyRISCV_TinyRISCV_H

#include "Utils/TinyRISCVBaseInfo.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
class TinyRISCVSubtarget;
class TinyRISCVTargetMachine;
class AsmPrinter;
class FunctionPass;
class MCInst;
class MCOperand;
class MachineInstr;
class MachineOperand;
class PassRegistry;

void LowerTinyRISCVMachineInstrToMCInst(const MachineInstr *MI, MCInst &OutMI,
                                    const AsmPrinter &AP);
bool LowerTinyRISCVMachineOperandToMCOperand(const MachineOperand &MO,
                                         MCOperand &MCOp, const AsmPrinter &AP);

FunctionPass *createTinyRISCVISelDag(TinyRISCVTargetMachine &TM);

FunctionPass *createTinyRISCVMergeBaseOffsetOptPass();
void initializeTinyRISCVMergeBaseOffsetOptPass(PassRegistry &);

FunctionPass *createTinyRISCVExpandPseudoPass();
void initializeTinyRISCVExpandPseudoPass(PassRegistry &);

}

#endif
