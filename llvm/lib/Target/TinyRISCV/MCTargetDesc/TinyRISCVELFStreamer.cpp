//===-- TinyRISCVELFStreamer.cpp - TinyRISCV ELF Target Streamer Methods ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides TinyRISCV specific target streamer methods.
//
//===----------------------------------------------------------------------===//

#include "TinyRISCVELFStreamer.h"
#include "MCTargetDesc/TinyRISCVAsmBackend.h"
#include "TinyRISCVMCTargetDesc.h"
#include "Utils/TinyRISCVBaseInfo.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCSubtargetInfo.h"

using namespace llvm;

// This part is for ELF object output.
TinyRISCVTargetELFStreamer::TinyRISCVTargetELFStreamer(MCStreamer &S,
                                               const MCSubtargetInfo &STI)
    : TinyRISCVTargetStreamer(S) {
  MCAssembler &MCA = getStreamer().getAssembler();
  const FeatureBitset &Features = STI.getFeatureBits();
  auto &MAB = static_cast<TinyRISCVAsmBackend &>(MCA.getBackend());
  TinyRISCVABI::ABI ABI = MAB.getTargetABI();
  assert(ABI != TinyRISCVABI::ABI_Unknown && "Improperly initialised target ABI");

  unsigned EFlags = MCA.getELFHeaderEFlags();

  switch (ABI) {
  case TinyRISCVABI::ABI_ILP32:
  case TinyRISCVABI::ABI_LP64:
    break;
  case TinyRISCVABI::ABI_Unknown:
    llvm_unreachable("Improperly initialised target ABI");
  }

  MCA.setELFHeaderEFlags(EFlags);
}

MCELFStreamer &TinyRISCVTargetELFStreamer::getStreamer() {
  return static_cast<MCELFStreamer &>(Streamer);
}

void TinyRISCVTargetELFStreamer::emitDirectiveOptionPush() {}
void TinyRISCVTargetELFStreamer::emitDirectiveOptionPop() {}
void TinyRISCVTargetELFStreamer::emitDirectiveOptionRVC() {}
void TinyRISCVTargetELFStreamer::emitDirectiveOptionNoRVC() {}
void TinyRISCVTargetELFStreamer::emitDirectiveOptionRelax() {}
void TinyRISCVTargetELFStreamer::emitDirectiveOptionNoRelax() {}
