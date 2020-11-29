//===-- TinyRISCVMCTargetDesc.cpp - TinyRISCV Target Descriptions -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// This file provides TinyRISCV-specific target descriptions.
///
//===----------------------------------------------------------------------===//

#include "TinyRISCVMCTargetDesc.h"
#include "TinyRISCVELFStreamer.h"
#include "TinyRISCVInstPrinter.h"
#include "TinyRISCVMCAsmInfo.h"
#include "TinyRISCVTargetStreamer.h"
#include "TargetInfo/TinyRISCVTargetInfo.h"
#include "Utils/TinyRISCVBaseInfo.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/Register.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"

#define GET_INSTRINFO_MC_DESC
#include "TinyRISCVGenInstrInfo.inc"

#define GET_REGINFO_MC_DESC
#include "TinyRISCVGenRegisterInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "TinyRISCVGenSubtargetInfo.inc"

using namespace llvm;

static MCInstrInfo *createTinyRISCVMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitTinyRISCVMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createTinyRISCVMCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitTinyRISCVMCRegisterInfo(X, TinyRISCV::X1);
  return X;
}

static MCAsmInfo *createTinyRISCVMCAsmInfo(const MCRegisterInfo &MRI,
                                       const Triple &TT,
                                       const MCTargetOptions &Options) {
  MCAsmInfo *MAI = new TinyRISCVMCAsmInfo(TT);

  Register SP = MRI.getDwarfRegNum(TinyRISCV::X2, true);
  MCCFIInstruction Inst = MCCFIInstruction::createDefCfa(nullptr, SP, 0);
  MAI->addInitialFrameState(Inst);

  return MAI;
}

static MCSubtargetInfo *createTinyRISCVMCSubtargetInfo(const Triple &TT,
                                                   StringRef CPU, StringRef FS) {
  std::string CPUName = CPU;
  if (CPUName.empty())
    CPUName = "generic";
  return createTinyRISCVMCSubtargetInfoImpl(TT, CPUName, FS);
}

static MCInstPrinter *createTinyRISCVMCInstPrinter(const Triple &T,
                                               unsigned SyntaxVariant,
                                               const MCAsmInfo &MAI,
                                               const MCInstrInfo &MII,
                                               const MCRegisterInfo &MRI) {
  return new TinyRISCVInstPrinter(MAI, MII, MRI);
}

static MCTargetStreamer *
createTinyRISCVObjectTargetStreamer(MCStreamer &S, const MCSubtargetInfo &STI) {
  const Triple &TT = STI.getTargetTriple();
  if (TT.isOSBinFormatELF())
    return new TinyRISCVTargetELFStreamer(S, STI);
  return nullptr;
}

static MCTargetStreamer *createTinyRISCVAsmTargetStreamer(MCStreamer &S,
                                                      formatted_raw_ostream &OS,
                                                      MCInstPrinter *InstPrint,
                                                      bool isVerboseAsm) {
  return new TinyRISCVTargetAsmStreamer(S, OS);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeTinyRISCVTargetMC() {
  Target *T = &getTheTinyRISCVTarget();
  TargetRegistry::RegisterMCAsmInfo(*T, createTinyRISCVMCAsmInfo);
  TargetRegistry::RegisterMCInstrInfo(*T, createTinyRISCVMCInstrInfo);
  TargetRegistry::RegisterMCRegInfo(*T, createTinyRISCVMCRegisterInfo);
  TargetRegistry::RegisterMCAsmBackend(*T, createTinyRISCVAsmBackend);
  TargetRegistry::RegisterMCCodeEmitter(*T, createTinyRISCVMCCodeEmitter);
  TargetRegistry::RegisterMCInstPrinter(*T, createTinyRISCVMCInstPrinter);
  TargetRegistry::RegisterMCSubtargetInfo(*T, createTinyRISCVMCSubtargetInfo);
  TargetRegistry::RegisterObjectTargetStreamer(
      *T, createTinyRISCVObjectTargetStreamer);

  // Register the asm target streamer.
  TargetRegistry::RegisterAsmTargetStreamer(*T, createTinyRISCVAsmTargetStreamer);
}
