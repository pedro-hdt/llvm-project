//===-- TinyRISCVMCAsmInfo.h - TinyRISCV Asm Info ----------------------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the TinyRISCVMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_TinyRISCV_MCTARGETDESC_TinyRISCVMCASMINFO_H
#define LLVM_LIB_TARGET_TinyRISCV_MCTARGETDESC_TinyRISCVMCASMINFO_H

#include "llvm/MC/MCAsmInfoELF.h"

namespace llvm {
class Triple;

class TinyRISCVMCAsmInfo : public MCAsmInfoELF {
  void anchor() override;

public:
  explicit TinyRISCVMCAsmInfo(const Triple &TargetTriple);

  const MCExpr *getExprForFDESymbol(const MCSymbol *Sym, unsigned Encoding,
                                    MCStreamer &Streamer) const override;
};

} // namespace llvm

#endif
