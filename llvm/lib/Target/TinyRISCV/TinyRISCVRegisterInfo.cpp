//
// Created by pedro-teixeira on 28/09/2020.
//

#include "TinyRISCVRegisterInfo.h"
#include "TinyRISCV.h"
#include "TinyRISCVSubtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#define GET_REGINFO_TARGET_DESC
#include "TinyRISCVGenRegisterInfo.inc"

using namespace llvm;

TinyRISCVRegisterInfo::TinyRISCVRegisterInfo() : TinyRISCVGenRegisterInfo(TinyRISCV::LR) {}

const MCPhysReg *
TinyRISCVRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return CC_Save_SaveList;
}

BitVector TinyRISCVRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved;
  // Use markSuperRegs to ensure any register aliases are also reserved
  markSuperRegs(Reserved, TinyRISCV::SP);
  markSuperRegs(Reserved, TinyRISCV::LR);
  return Reserved;
}

void TinyRISCVRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                          int SPAdj, unsigned int FIOperandNum,
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
    // TII->movImm(MBB, II, DL, ScratchReg, Offset);
    BuildMI(MBB, II, DL, TII->get(TinyRISCV::ADDrr), ScratchReg)
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
  return TinyRISCV::X2;
}
