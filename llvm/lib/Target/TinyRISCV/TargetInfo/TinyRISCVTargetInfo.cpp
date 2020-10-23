//
// Created by pedro-teixeira on 23/09/2020.
//

#include "TargetInfo/TinyRISCVTargetInfo.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

Target &llvm::getTheTinyRISCVTarget() {
  static Target TheTinyRISCVTarget;
  return TheTinyRISCVTarget;
}
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeTinyRISCVTargetInfo() {
  RegisterTarget<Triple::tinyriscv> X(getTheTinyRISCVTarget(), "toy",
                                "A practice target", "TinyRISCV");
}