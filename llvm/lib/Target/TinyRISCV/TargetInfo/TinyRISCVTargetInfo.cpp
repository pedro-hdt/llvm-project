//===-- TinyRISCVTargetInfo.cpp - TinyRISCV Target Implementation -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "TargetInfo/TinyRISCVTargetInfo.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

Target &llvm::getTheTinyRISCVTarget() {
  static Target TheTinyRISCVTarget;
  return TheTinyRISCVTarget;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeTinyRISCVTargetInfo() {
  RegisterTarget<Triple::tinyriscv> X(getTheTinyRISCVTarget(), "tinyriscv",
                                    "TinyRISCV toy backend", "TinyRISCV");
}
