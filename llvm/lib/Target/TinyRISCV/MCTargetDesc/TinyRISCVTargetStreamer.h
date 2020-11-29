//===-- TinyRISCVTargetStreamer.h - TinyRISCV Target Streamer ----------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_TinyRISCV_TinyRISCVTARGETSTREAMER_H
#define LLVM_LIB_TARGET_TinyRISCV_TinyRISCVTARGETSTREAMER_H

#include "llvm/MC/MCStreamer.h"

namespace llvm {

class TinyRISCVTargetStreamer : public MCTargetStreamer {
public:
  TinyRISCVTargetStreamer(MCStreamer &S);

  virtual void emitDirectiveOptionPush() = 0;
  virtual void emitDirectiveOptionPop() = 0;
  virtual void emitDirectiveOptionRVC() = 0;
  virtual void emitDirectiveOptionNoRVC() = 0;
  virtual void emitDirectiveOptionRelax() = 0;
  virtual void emitDirectiveOptionNoRelax() = 0;
};

// This part is for ascii assembly output
class TinyRISCVTargetAsmStreamer : public TinyRISCVTargetStreamer {
  formatted_raw_ostream &OS;

public:
  TinyRISCVTargetAsmStreamer(MCStreamer &S, formatted_raw_ostream &OS);

  void emitDirectiveOptionPush() override;
  void emitDirectiveOptionPop() override;
  void emitDirectiveOptionRVC() override;
  void emitDirectiveOptionNoRVC() override;
  void emitDirectiveOptionRelax() override;
  void emitDirectiveOptionNoRelax() override;
};

}
#endif
