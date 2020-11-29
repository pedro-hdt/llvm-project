//===-- TinyRISCVELFStreamer.h - TinyRISCV ELF Target Streamer ---------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_TinyRISCV_TinyRISCVELFSTREAMER_H
#define LLVM_LIB_TARGET_TinyRISCV_TinyRISCVELFSTREAMER_H

#include "TinyRISCVTargetStreamer.h"
#include "llvm/MC/MCELFStreamer.h"

namespace llvm {

class TinyRISCVTargetELFStreamer : public TinyRISCVTargetStreamer {
public:
  MCELFStreamer &getStreamer();
  TinyRISCVTargetELFStreamer(MCStreamer &S, const MCSubtargetInfo &STI);

  virtual void emitDirectiveOptionPush();
  virtual void emitDirectiveOptionPop();
  virtual void emitDirectiveOptionRVC();
  virtual void emitDirectiveOptionNoRVC();
  virtual void emitDirectiveOptionRelax();
  virtual void emitDirectiveOptionNoRelax();
};
}
#endif
