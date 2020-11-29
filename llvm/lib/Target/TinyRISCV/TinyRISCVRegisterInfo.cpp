//===-- TinyRISCVRegisterInfo.cpp - TinyRISCV Register Information ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the TinyRISCV implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#include "TinyRISCVRegisterInfo.h"
#include "TinyRISCV.h"
#include "TinyRISCVSubtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/Support/ErrorHandling.h"

#define GET_REGINFO_TARGET_DESC
#include "TinyRISCVGenRegisterInfo.inc"

using namespace llvm;

static_assert(TinyRISCV::X1 == TinyRISCV::X0 + 1, "Register list not consecutive");
static_assert(TinyRISCV::X31 == TinyRISCV::X0 + 31, "Register list not consecutive");

TinyRISCVRegisterInfo::TinyRISCVRegisterInfo(unsigned HwMode)
    : TinyRISCVGenRegisterInfo(TinyRISCV::X1, /*DwarfFlavour*/0, /*EHFlavor*/0,
                           /*PC*/0, HwMode) {}

const MCPhysReg *
TinyRISCVRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return CSR_TinyRISCV_SaveList;
}

BitVector TinyRISCVRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  const TinyRISCVFrameLowering *TFI = getFrameLowering(MF);
  BitVector Reserved(getNumRegs());

  // Mark any registers requested to be reserved as such
  for (size_t Reg = 0; Reg < getNumRegs(); Reg++) {
    if (MF.getSubtarget<TinyRISCVSubtarget>().isRegisterReservedByUser(Reg))
      markSuperRegs(Reserved, Reg);
  }

  // Use markSuperRegs to ensure any register aliases are also reserved
  markSuperRegs(Reserved, TinyRISCV::X0); // zero
  markSuperRegs(Reserved, TinyRISCV::X2); // sp
  markSuperRegs(Reserved, TinyRISCV::X3); // gp
  markSuperRegs(Reserved, TinyRISCV::X4); // tp
  if (TFI->hasFP(MF))
    markSuperRegs(Reserved, TinyRISCV::X8); // fp
  // Reserve the base register if we need to realign the stack and allocate
  // variable-sized objects at runtime.
  if (TFI->hasBP(MF))
    markSuperRegs(Reserved, TinyRISCVABI::getBPReg()); // bp
  assert(checkAllSuperRegsMarked(Reserved));
  return Reserved;
}

bool TinyRISCVRegisterInfo::isAsmClobberable(const MachineFunction &MF,
                                         unsigned PhysReg) const {
  return !MF.getSubtarget<TinyRISCVSubtarget>().isRegisterReservedByUser(PhysReg);
}

bool TinyRISCVRegisterInfo::isConstantPhysReg(unsigned PhysReg) const {
  return PhysReg == TinyRISCV::X0;
}

void TinyRISCVRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                            int SPAdj, unsigned FIOperandNum,
                                            RegScavenger *RS) const {
  assert(SPAdj == 0 && "Unexpected non-zero SPAdj value");

  MachineInstr &MI = *II;
  MachineFunction &MF = *MI.getParent()->getParent();
  MachineRegisterInfo &MRI = MF.getRegInfo();
  const TinyRISCVInstrInfo *TII = MF.getSubtarget<TinyRISCVSubtarget>().getInstrInfo();
  DebugLoc DL = MI.getDebugLoc();

  int FrameIndex = MI.getOperand(FIOperandNum).getIndex();
  unsigned FrameReg;
  int Offset =
      getFrameLowering(MF)->getFrameIndexReference(MF, FrameIndex, FrameReg) +
      MI.getOperand(FIOperandNum + 1).getImm();

  if (!isInt<32>(Offset)) {
    report_fatal_error(
        "Frame offsets outside of the signed 32-bit range not supported");
  }

  MachineBasicBlock &MBB = *MI.getParent();
  bool FrameRegIsKill = false;

  if (!isInt<12>(Offset)) {
    assert(isInt<32>(Offset) && "Int32 expected");
    // The offset won't fit in an immediate, so use a scratch register instead
    // Modify Offset and FrameReg appropriately
    Register ScratchReg = MRI.createVirtualRegister(&TinyRISCV::GPRRegClass);
    TII->movImm(MBB, II, DL, ScratchReg, Offset);
    BuildMI(MBB, II, DL, TII->get(TinyRISCV::ADD), ScratchReg)
        .addReg(FrameReg)
        .addReg(ScratchReg, RegState::Kill);
    Offset = 0;
    FrameReg = ScratchReg;
    FrameRegIsKill = true;
  }

  MI.getOperand(FIOperandNum)
      .ChangeToRegister(FrameReg, false, false, FrameRegIsKill);
  MI.getOperand(FIOperandNum + 1).ChangeToImmediate(Offset);
}

Register TinyRISCVRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  const TargetFrameLowering *TFI = getFrameLowering(MF);
  return TFI->hasFP(MF) ? TinyRISCV::X8 : TinyRISCV::X2;
}

const uint32_t *
TinyRISCVRegisterInfo::getCallPreservedMask(const MachineFunction & MF,
                                        CallingConv::ID /*CC*/) const {
  return CSR_TinyRISCV_RegMask;
}
