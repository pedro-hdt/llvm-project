//
// Created by pedro-teixeira on 20/09/2020.
//

#include "TinyRISCVFrameLowering.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "TinyRISCVSubtarget.h"

using namespace llvm;


void TinyRISCVFrameLowering::emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB)
    const {
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
  MachineBasicBlock::iterator MBBI = MBB.begin();
  DebugLoc dl = MBBI != MBB.end() ? MBBI->getDebugLoc() :
                DebugLoc();
  uint64_t StackSize = computeStackSize(MF);
  if (!StackSize) {
    return;
  }
  unsigned StackReg = TinyRISCV::SP;
  unsigned OffsetReg = materializeOffset(MF, MBB, MBBI,
                                         (unsigned)StackSize);
  if (OffsetReg) {
    BuildMI(MBB, MBBI, dl, TII.get(TinyRISCV::ADDrr), StackReg)
        .addReg(StackReg)
        .addReg(OffsetReg)
        .setMIFlag(MachineInstr::FrameSetup);
  } else {
    llvm_unreachable("Need ADDri implemented to deal with this");
//    BuildMI(MBB, MBBI, dl, TII.get(TinyRISCV::SUBri), StackReg)
//        .addReg(StackReg)
//        .addImm(StackSize)
//        .setMIFlag(MachineInstr::FrameSetup);
  }
}

void TinyRISCVFrameLowering::emitEpilogue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const {
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  DebugLoc dl = MBBI->getDebugLoc();
  uint64_t StackSize = computeStackSize(MF);
  if (!StackSize) {
    return;
  }
  unsigned StackReg = TinyRISCV::SP;
  unsigned OffsetReg = materializeOffset(MF, MBB, MBBI, (unsigned)StackSize);
  if (OffsetReg) {
    BuildMI(MBB, MBBI, dl, TII.get(TinyRISCV::ADDrr), StackReg)
        .addReg(StackReg)
        .addReg(OffsetReg)
        .setMIFlag(MachineInstr::FrameSetup);
  } else {
    llvm_unreachable("Need ADDri implemented to deal with this");
//    BuildMI(MBB, MBBI, dl, TII.get(TinyRISCV::ADDri), StackReg)
//        .addReg(StackReg)
//        .addImm(StackSize)
//        .setMIFlag(MachineInstr::FrameSetup);
  }
}

unsigned TinyRISCVFrameLowering::materializeOffset(MachineFunction &MF,
                                             MachineBasicBlock &MBB,
                                             MachineBasicBlock::iterator MBBI,
                                             unsigned int Offset) {
  const TargetInstrInfo &TII =
      *MF.getSubtarget().getInstrInfo();
  DebugLoc dl = MBBI != MBB.end() ? MBBI->getDebugLoc() :
                DebugLoc();
  const uint64_t MaxSubImm = 0xfff;
  if (Offset <= MaxSubImm) {
    return 0;
  } else {
    unsigned OffsetReg = TinyRISCV::R2;
    unsigned OffsetLo = (unsigned)(Offset & 0xffff);
    unsigned OffsetHi = (unsigned)((Offset & 0xffff0000) >>
                                                         16);
//    BuildMI(MBB, MBBI, dl, TII.get(TinyRISCV::MOVLOi16),
//            OffsetReg)
//        .addImm(OffsetLo)
//        .setMIFlag(MachineInstr::FrameSetup);
//    if (OffsetHi) {
//      BuildMI(MBB, MBBI, dl, TII.get(TinyRISCV::MOVHIi16),
//              OffsetReg)
//          .addReg(OffsetReg)
//          .addImm(OffsetHi)
//          .setMIFlag(MachineInstr::FrameSetup);
//    }
//    return OffsetReg;
  }
}

uint64_t TinyRISCVFrameLowering::computeStackSize(MachineFunction &MF) const {
  MachineFrameInfo &MFI = MF.getFrameInfo();
  uint64_t StackSize = MFI.getStackSize();
  unsigned StackAlign = getStackAlignment();
  if (StackAlign > 0) {
    StackSize = alignTo(StackSize, StackAlign);
  }
  return StackSize;
}