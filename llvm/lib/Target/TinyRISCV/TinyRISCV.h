//
// Created by pedro-teixeira on 23/09/2020.
//

#ifndef LLVM_LIB_TARGET_TinyRISCV_H
#define LLVM_LIB_TARGET_TinyRISCV_H

#include "MCTargetDesc/TinyRISCVMCTargetDesc.h"
#include "llvm/Target/TargetMachine.h"
namespace llvm {
class TargetMachine;
class TinyRISCVTargetMachine;

FunctionPass *createTinyRISCVISelDag(TinyRISCVTargetMachine &TM);

void LowerTinyRISCVMachineInstrToMCInst(const MachineInstr *MI, MCInst &OutMI,
                                    const AsmPrinter &AP);
bool LowerTinyRISCVMachineOperandToMCOperand(const MachineOperand &MO,
                                         MCOperand &MCOp, const AsmPrinter &AP);

} // end namespace llvm;

#endif // LLVM_LIB_TARGET_TinyRISCV_H
