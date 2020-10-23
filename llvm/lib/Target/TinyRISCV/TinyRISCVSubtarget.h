//
// Created by pedro-teixeira on 23/09/2020.
//

#ifndef LLVM_LIB_TARGET_TinyRISCV_TinyRISCVSUBTARGET_H
#define LLVM_LIB_TARGET_TinyRISCV_TinyRISCVSUBTARGET_H

#include "TinyRISCVFrameLowering.h"
#include "TinyRISCVISelLowering.h"
#include "TinyRISCVInstrInfo.h"
#include "llvm/CodeGen/SelectionDAGTargetInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Target/TargetMachine.h"

#define GET_SUBTARGETINFO_HEADER
#include "TinyRISCVGenSubtargetInfo.inc"

namespace llvm {
class StringRef;

class TinyRISCVSubtarget : public TinyRISCVGenSubtargetInfo {
  virtual void anchor();
  TinyRISCVFrameLowering FrameLowering;
  TinyRISCVInstrInfo InstrInfo;
  TinyRISCVRegisterInfo RegInfo;
  TinyRISCVTargetLowering TLInfo;
  SelectionDAGTargetInfo TSInfo;

public:
  // Initializes the data members to match that of the specified triple.
  TinyRISCVSubtarget(const Triple &TT, StringRef CPU, StringRef FS,
               const TargetMachine &TM);

  const TinyRISCVFrameLowering *getFrameLowering() const override {
    return &FrameLowering;
  }
  const TinyRISCVInstrInfo *getInstrInfo() const override { return &InstrInfo; }
  const TinyRISCVRegisterInfo *getRegisterInfo() const override {
    return &RegInfo;
  }
  const TinyRISCVTargetLowering *getTargetLowering() const override {
    return &TLInfo;
  }
  const SelectionDAGTargetInfo *getSelectionDAGInfo() const override {
    return &TSInfo;
  }
};
} // End llvm namespace

#endif // LLVM_LIB_TARGET_TinyRISCV_TinyRISCVSUBTARGET_H
