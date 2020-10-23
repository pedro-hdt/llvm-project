//
// Created by pedro-teixeira on 28/09/2020.
//

#include "TinyRISCV.h"
#include "TinyRISCVISelLowering.h"
#include "TinyRISCVSubtarget.h"
using namespace llvm;

#define DEBUG_TYPE "tinyriscv-lower"

TinyRISCVTargetLowering::TinyRISCVTargetLowering(const TargetMachine &TM,
                                     const TinyRISCVSubtarget &STI)
    : TargetLowering(TM), Subtarget(&STI) {

  addRegisterClass(MVT::i32, &TinyRISCV::GPRRegClass);

  // Compute derived properties from the register classes.
  computeRegisterProperties(STI.getRegisterInfo());

  setStackPointerRegisterToSaveRestore(TinyRISCV::SP);

  // TODO this is probably incomplete, even for a single instruction target

}
