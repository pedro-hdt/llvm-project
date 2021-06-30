//===-- TinyRISCVSubtarget.cpp - TinyRISCV Subtarget Information ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the TinyRISCV specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "TinyRISCVSubtarget.h"
#include "TinyRISCV.h"
#include "TinyRISCVFrameLowering.h"
#include "TinyRISCVTargetMachine.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define DEBUG_TYPE "tinyriscv-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "TinyRISCVGenSubtargetInfo.inc"

void TinyRISCVSubtarget::anchor() {}

TinyRISCVSubtarget &TinyRISCVSubtarget::initializeSubtargetDependencies(
    const Triple &TT, StringRef CPU, StringRef FS, StringRef ABIName) {
  // Determine default and user-specified characteristics
  bool Is64Bit = TT.isArch64Bit();
  std::string CPUName = CPU;
  if (CPUName.empty())
    CPUName = "generic";
  ParseSubtargetFeatures(CPUName, FS);
  if (Is64Bit) {
    XLenVT = MVT::i64;
    XLen = 64;
  }

  TargetABI = TinyRISCVABI::computeTargetABI(TT, getFeatureBits(), ABIName);
  TinyRISCVFeatures::validate(TT, getFeatureBits());
  return *this;
}

TinyRISCVSubtarget::TinyRISCVSubtarget(const Triple &TT, StringRef CPU, StringRef FS,
                               StringRef ABIName, const TargetMachine &TM)
    : TinyRISCVGenSubtargetInfo(TT, CPU, FS),
      UserReservedRegister(TinyRISCV::NUM_TARGET_REGS),
      FrameLowering(initializeSubtargetDependencies(TT, CPU, FS, ABIName)),
      InstrInfo(*this), RegInfo(getHwMode()), TLInfo(TM, *this) {}
