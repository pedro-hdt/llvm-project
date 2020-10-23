//
// Created by pedro-teixeira on 23/09/2020.
//

#ifndef LLVM_LIB_TARGET_TinyRISCV_MCTARGETDESC_TinyRISCVMCTARGETDESC_H
#define LLVM_LIB_TARGET_TinyRISCV_MCTARGETDESC_TinyRISCVMCTARGETDESC_H

#include "llvm/Support/DataTypes.h"
#include <llvm/CodeGen/AsmPrinter.h>
namespace llvm {
class Target;
class MCInstrInfo;
class MCRegisterInfo;
class MCSubtargetInfo;
class MCContext;
class MCCodeEmitter;
class MCAsmInfo;
class MCCodeGenInfo;
class MCInstPrinter;
class MCObjectWriter;
class MCAsmBackend;
class StringRef;
class raw_ostream;
extern Target TheTinyRISCVTarget;

MCCodeEmitter *createTinyRISCVMCCodeEmitter(const MCInstrInfo &MCII,
                                      const MCRegisterInfo &MRI,
                                      MCContext &Ctx);
void LowerTinyRISCVMachineInstrToMCInst(const MachineInstr *MI, MCInst &OutMI,
                                  const llvm::AsmPrinter &AP);
} // end namespace llvm

#define GET_REGINFO_ENUM
#include "TinyRISCVGenRegisterInfo.inc"

#define GET_INSTRINFO_ENUM
#include "TinyRISCVGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "TinyRISCVGenSubtargetInfo.inc"

#endif // LLVM_LIB_TARGET_TinyRISCV_MCTARGETDESC_TinyRISCVMCTARGETDESC_H
