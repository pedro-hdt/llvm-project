//===- TinyRISCVTargetTransformInfo.h - RISC-V specific TTI ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
/// \file
/// This file defines a TargetTransformInfo::Concept conforming object specific
/// to the RISC-V target machine. It uses the target's detailed information to
/// provide more precise answers to certain TTI queries, while letting the
/// target independent and default TTI implementations handle the rest.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_TinyRISCV_TinyRISCVTARGETTRANSFORMINFO_H
#define LLVM_LIB_TARGET_TinyRISCV_TinyRISCVTARGETTRANSFORMINFO_H

#include "TinyRISCVSubtarget.h"
#include "TinyRISCVTargetMachine.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/CodeGen/BasicTTIImpl.h"
#include "llvm/IR/Function.h"

namespace llvm {

class TinyRISCVTTIImpl : public BasicTTIImplBase<TinyRISCVTTIImpl> {
  using BaseT = BasicTTIImplBase<TinyRISCVTTIImpl>;
  using TTI = TargetTransformInfo;

  friend BaseT;

  const TinyRISCVSubtarget *ST;
  const TinyRISCVTargetLowering *TLI;

  const TinyRISCVSubtarget *getST() const { return ST; }
  const TinyRISCVTargetLowering *getTLI() const { return TLI; }

public:
  explicit TinyRISCVTTIImpl(const TinyRISCVTargetMachine *TM, const Function &F)
      : BaseT(TM, F.getParent()->getDataLayout()), ST(TM->getSubtargetImpl(F)),
        TLI(ST->getTargetLowering()) {}

  int getIntImmCost(const APInt &Imm, Type *Ty);
  int getIntImmCostInst(unsigned Opcode, unsigned Idx, const APInt &Imm, Type *Ty);
  int getIntImmCostIntrin(Intrinsic::ID IID, unsigned Idx, const APInt &Imm,
                          Type *Ty);
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_TinyRISCV_TinyRISCVTARGETTRANSFORMINFO_H