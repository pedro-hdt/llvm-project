//
// Created by pedro-teixeira on 28/09/2020.
//

#ifndef LLVM_LIB_TARGET_TinyRISCV_TinyRISCVISELLOWERING_H
#define LLVM_LIB_TARGET_TinyRISCV_TinyRISCVISELLOWERING_H

#include "TinyRISCV.h"
#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {

class TinyRISCVSubtarget;

class TinyRISCVTargetLowering : public TargetLowering {
  const TinyRISCVSubtarget *Subtarget;
public:
  TinyRISCVTargetLowering(const TargetMachine &TM, const TinyRISCVSubtarget &STI);
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_TinyRISCV_TinyRISCVISELLOWERING_H
