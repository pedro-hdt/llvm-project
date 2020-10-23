//
// Created by pedro-teixeira on 28/09/2020.
//

#ifndef LLVM_LIB_TARGET_TinyRISCV_TinyRISCVREGISTERINFO_H
#define LLVM_LIB_TARGET_TinyRISCV_TinyRISCVREGISTERINFO_H

#include "llvm/CodeGen/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "TinyRISCVGenRegisterInfo.inc"

namespace llvm {
struct TinyRISCVRegisterInfo : public TinyRISCVGenRegisterInfo {
  TinyRISCVRegisterInfo();

  const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const override;

  virtual BitVector getReservedRegs(const MachineFunction &MF) const override;

  void eliminateFrameIndex(MachineBasicBlock::iterator MI, int SPAdj,
                           unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override;

  Register getFrameRegister(const MachineFunction &MF) const override;

};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_TinyRISCV_TinyRISCVREGISTERINFO_H
