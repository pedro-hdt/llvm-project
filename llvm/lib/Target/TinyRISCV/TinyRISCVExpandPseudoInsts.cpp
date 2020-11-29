//===-- TinyRISCVExpandPseudoInsts.cpp - Expand pseudo instructions -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains a pass that expands pseudo instructions into target
// instructions. This pass should be run after register allocation but before
// the post-regalloc scheduling pass.
//
//===----------------------------------------------------------------------===//

#include "TinyRISCV.h"
#include "TinyRISCVInstrInfo.h"
#include "TinyRISCVTargetMachine.h"

#include "llvm/CodeGen/LivePhysRegs.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

using namespace llvm;

#define TinyRISCV_EXPAND_PSEUDO_NAME "TinyRISCV pseudo instruction expansion pass"

namespace {

class TinyRISCVExpandPseudo : public MachineFunctionPass {
public:
  const TinyRISCVInstrInfo *TII;
  static char ID;

  TinyRISCVExpandPseudo() : MachineFunctionPass(ID) {
    initializeTinyRISCVExpandPseudoPass(*PassRegistry::getPassRegistry());
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

  StringRef getPassName() const override { return TinyRISCV_EXPAND_PSEUDO_NAME; }

private:
  bool expandMBB(MachineBasicBlock &MBB);
  bool expandMI(MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
                MachineBasicBlock::iterator &NextMBBI);
  bool expandAtomicBinOp(MachineBasicBlock &MBB,
                         MachineBasicBlock::iterator MBBI, AtomicRMWInst::BinOp,
                         bool IsMasked, int Width,
                         MachineBasicBlock::iterator &NextMBBI);
  bool expandAtomicMinMaxOp(MachineBasicBlock &MBB,
                            MachineBasicBlock::iterator MBBI,
                            AtomicRMWInst::BinOp, bool IsMasked, int Width,
                            MachineBasicBlock::iterator &NextMBBI);
  bool expandAtomicCmpXchg(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator MBBI, bool IsMasked,
                           int Width, MachineBasicBlock::iterator &NextMBBI);
  bool expandAuipcInstPair(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator MBBI,
                           MachineBasicBlock::iterator &NextMBBI,
                           unsigned FlagsHi, unsigned SecondOpcode);
  bool expandLoadLocalAddress(MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator MBBI,
                              MachineBasicBlock::iterator &NextMBBI);
  bool expandLoadAddress(MachineBasicBlock &MBB,
                         MachineBasicBlock::iterator MBBI,
                         MachineBasicBlock::iterator &NextMBBI);
  bool expandLoadTLSIEAddress(MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator MBBI,
                              MachineBasicBlock::iterator &NextMBBI);
  bool expandLoadTLSGDAddress(MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator MBBI,
                              MachineBasicBlock::iterator &NextMBBI);
};

char TinyRISCVExpandPseudo::ID = 0;

bool TinyRISCVExpandPseudo::runOnMachineFunction(MachineFunction &MF) {
  TII = static_cast<const TinyRISCVInstrInfo *>(MF.getSubtarget().getInstrInfo());
  bool Modified = false;
  for (auto &MBB : MF)
    Modified |= expandMBB(MBB);
  return Modified;
}

bool TinyRISCVExpandPseudo::expandMBB(MachineBasicBlock &MBB) {
  bool Modified = false;

  MachineBasicBlock::iterator MBBI = MBB.begin(), E = MBB.end();
  while (MBBI != E) {
    MachineBasicBlock::iterator NMBBI = std::next(MBBI);
    Modified |= expandMI(MBB, MBBI, NMBBI);
    MBBI = NMBBI;
  }

  return Modified;
}

bool TinyRISCVExpandPseudo::expandMI(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator MBBI,
                                 MachineBasicBlock::iterator &NextMBBI) {
  return false;
}

static unsigned getLRForRMW32(AtomicOrdering Ordering) {
  llvm_unreachable("Unexpected AtomicOrdering");
}

static unsigned getSCForRMW32(AtomicOrdering Ordering) {
  llvm_unreachable("Unexpected AtomicOrdering");
}

static unsigned getLRForRMW64(AtomicOrdering Ordering) {
  llvm_unreachable("Unexpected AtomicOrdering");
}

static unsigned getSCForRMW64(AtomicOrdering Ordering) {
  llvm_unreachable("Unexpected AtomicOrdering");
}

static unsigned getLRForRMW(AtomicOrdering Ordering, int Width) {
  if (Width == 32)
    return getLRForRMW32(Ordering);
  if (Width == 64)
    return getLRForRMW64(Ordering);
  llvm_unreachable("Unexpected LR width\n");
}

static unsigned getSCForRMW(AtomicOrdering Ordering, int Width) {
  if (Width == 32)
    return getSCForRMW32(Ordering);
  if (Width == 64)
    return getSCForRMW64(Ordering);
  llvm_unreachable("Unexpected SC width\n");
}

static void doAtomicBinOpExpansion(const TinyRISCVInstrInfo *TII, MachineInstr &MI,
                                   DebugLoc DL, MachineBasicBlock *ThisMBB,
                                   MachineBasicBlock *LoopMBB,
                                   MachineBasicBlock *DoneMBB,
                                   AtomicRMWInst::BinOp BinOp, int Width) {
  Register DestReg = MI.getOperand(0).getReg();
  Register ScratchReg = MI.getOperand(1).getReg();
  Register AddrReg = MI.getOperand(2).getReg();
  Register IncrReg = MI.getOperand(3).getReg();
  AtomicOrdering Ordering =
      static_cast<AtomicOrdering>(MI.getOperand(4).getImm());

  // .loop:
  //   lr.[w|d] dest, (addr)
  //   binop scratch, dest, val
  //   sc.[w|d] scratch, scratch, (addr)
  //   bnez scratch, loop
  BuildMI(LoopMBB, DL, TII->get(getLRForRMW(Ordering, Width)), DestReg)
      .addReg(AddrReg);
  switch (BinOp) {
  default:
    llvm_unreachable("Unexpected AtomicRMW BinOp");
  case AtomicRMWInst::Nand:
    BuildMI(LoopMBB, DL, TII->get(TinyRISCV::AND), ScratchReg)
        .addReg(DestReg)
        .addReg(IncrReg);
    BuildMI(LoopMBB, DL, TII->get(TinyRISCV::XORI), ScratchReg)
        .addReg(ScratchReg)
        .addImm(-1);
    break;
  }
  BuildMI(LoopMBB, DL, TII->get(getSCForRMW(Ordering, Width)), ScratchReg)
      .addReg(AddrReg)
      .addReg(ScratchReg);
  BuildMI(LoopMBB, DL, TII->get(TinyRISCV::BNE))
      .addReg(ScratchReg)
      .addReg(TinyRISCV::X0)
      .addMBB(LoopMBB);
}

static void insertMaskedMerge(const TinyRISCVInstrInfo *TII, DebugLoc DL,
                              MachineBasicBlock *MBB, Register DestReg,
                              Register OldValReg, Register NewValReg,
                              Register MaskReg, Register ScratchReg) {
  assert(OldValReg != ScratchReg && "OldValReg and ScratchReg must be unique");
  assert(OldValReg != MaskReg && "OldValReg and MaskReg must be unique");
  assert(ScratchReg != MaskReg && "ScratchReg and MaskReg must be unique");

  // We select bits from newval and oldval using:
  // https://graphics.stanford.edu/~seander/bithacks.html#MaskedMerge
  // r = oldval ^ ((oldval ^ newval) & masktargetdata);
  BuildMI(MBB, DL, TII->get(TinyRISCV::XOR), ScratchReg)
      .addReg(OldValReg)
      .addReg(NewValReg);
  BuildMI(MBB, DL, TII->get(TinyRISCV::AND), ScratchReg)
      .addReg(ScratchReg)
      .addReg(MaskReg);
  BuildMI(MBB, DL, TII->get(TinyRISCV::XOR), DestReg)
      .addReg(OldValReg)
      .addReg(ScratchReg);
}

static void doMaskedAtomicBinOpExpansion(
    const TinyRISCVInstrInfo *TII, MachineInstr &MI, DebugLoc DL,
    MachineBasicBlock *ThisMBB, MachineBasicBlock *LoopMBB,
    MachineBasicBlock *DoneMBB, AtomicRMWInst::BinOp BinOp, int Width) {
  assert(Width == 32 && "Should never need to expand masked 64-bit operations");
  Register DestReg = MI.getOperand(0).getReg();
  Register ScratchReg = MI.getOperand(1).getReg();
  Register AddrReg = MI.getOperand(2).getReg();
  Register IncrReg = MI.getOperand(3).getReg();
  Register MaskReg = MI.getOperand(4).getReg();
  AtomicOrdering Ordering =
      static_cast<AtomicOrdering>(MI.getOperand(5).getImm());

  // .loop:
  //   lr.w destreg, (alignedaddr)
  //   binop scratch, destreg, incr
  //   xor scratch, destreg, scratch
  //   and scratch, scratch, masktargetdata
  //   xor scratch, destreg, scratch
  //   sc.w scratch, scratch, (alignedaddr)
  //   bnez scratch, loop
  BuildMI(LoopMBB, DL, TII->get(getLRForRMW32(Ordering)), DestReg)
      .addReg(AddrReg);
  switch (BinOp) {
  default:
    llvm_unreachable("Unexpected AtomicRMW BinOp");
  case AtomicRMWInst::Xchg:
    BuildMI(LoopMBB, DL, TII->get(TinyRISCV::ADDI), ScratchReg)
        .addReg(IncrReg)
        .addImm(0);
    break;
  case AtomicRMWInst::Add:
    BuildMI(LoopMBB, DL, TII->get(TinyRISCV::ADD), ScratchReg)
        .addReg(DestReg)
        .addReg(IncrReg);
    break;
  case AtomicRMWInst::Sub:
    BuildMI(LoopMBB, DL, TII->get(TinyRISCV::SUB), ScratchReg)
        .addReg(DestReg)
        .addReg(IncrReg);
    break;
  case AtomicRMWInst::Nand:
    BuildMI(LoopMBB, DL, TII->get(TinyRISCV::AND), ScratchReg)
        .addReg(DestReg)
        .addReg(IncrReg);
    BuildMI(LoopMBB, DL, TII->get(TinyRISCV::XORI), ScratchReg)
        .addReg(ScratchReg)
        .addImm(-1);
    break;
  }

  insertMaskedMerge(TII, DL, LoopMBB, ScratchReg, DestReg, ScratchReg, MaskReg,
                    ScratchReg);

  BuildMI(LoopMBB, DL, TII->get(getSCForRMW32(Ordering)), ScratchReg)
      .addReg(AddrReg)
      .addReg(ScratchReg);
  BuildMI(LoopMBB, DL, TII->get(TinyRISCV::BNE))
      .addReg(ScratchReg)
      .addReg(TinyRISCV::X0)
      .addMBB(LoopMBB);
}

bool TinyRISCVExpandPseudo::expandAtomicBinOp(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
    AtomicRMWInst::BinOp BinOp, bool IsMasked, int Width,
    MachineBasicBlock::iterator &NextMBBI) {
  MachineInstr &MI = *MBBI;
  DebugLoc DL = MI.getDebugLoc();

  MachineFunction *MF = MBB.getParent();
  auto LoopMBB = MF->CreateMachineBasicBlock(MBB.getBasicBlock());
  auto DoneMBB = MF->CreateMachineBasicBlock(MBB.getBasicBlock());

  // Insert new MBBs.
  MF->insert(++MBB.getIterator(), LoopMBB);
  MF->insert(++LoopMBB->getIterator(), DoneMBB);

  // Set up successors and transfer remaining instructions to DoneMBB.
  LoopMBB->addSuccessor(LoopMBB);
  LoopMBB->addSuccessor(DoneMBB);
  DoneMBB->splice(DoneMBB->end(), &MBB, MI, MBB.end());
  DoneMBB->transferSuccessors(&MBB);
  MBB.addSuccessor(LoopMBB);

  if (!IsMasked)
    doAtomicBinOpExpansion(TII, MI, DL, &MBB, LoopMBB, DoneMBB, BinOp, Width);
  else
    doMaskedAtomicBinOpExpansion(TII, MI, DL, &MBB, LoopMBB, DoneMBB, BinOp,
                                 Width);

  NextMBBI = MBB.end();
  MI.eraseFromParent();

  LivePhysRegs LiveRegs;
  computeAndAddLiveIns(LiveRegs, *LoopMBB);
  computeAndAddLiveIns(LiveRegs, *DoneMBB);

  return true;
}

static void insertSext(const TinyRISCVInstrInfo *TII, DebugLoc DL,
                       MachineBasicBlock *MBB, Register ValReg,
                       Register ShamtReg) {
  BuildMI(MBB, DL, TII->get(TinyRISCV::SLL), ValReg)
      .addReg(ValReg)
      .addReg(ShamtReg);
  BuildMI(MBB, DL, TII->get(TinyRISCV::SRA), ValReg)
      .addReg(ValReg)
      .addReg(ShamtReg);
}

bool TinyRISCVExpandPseudo::expandAtomicMinMaxOp(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
    AtomicRMWInst::BinOp BinOp, bool IsMasked, int Width,
    MachineBasicBlock::iterator &NextMBBI) {
  assert(IsMasked == true &&
         "Should only need to expand masked atomic max/min");
  assert(Width == 32 && "Should never need to expand masked 64-bit operations");

  MachineInstr &MI = *MBBI;
  DebugLoc DL = MI.getDebugLoc();
  MachineFunction *MF = MBB.getParent();
  auto LoopHeadMBB = MF->CreateMachineBasicBlock(MBB.getBasicBlock());
  auto LoopIfBodyMBB = MF->CreateMachineBasicBlock(MBB.getBasicBlock());
  auto LoopTailMBB = MF->CreateMachineBasicBlock(MBB.getBasicBlock());
  auto DoneMBB = MF->CreateMachineBasicBlock(MBB.getBasicBlock());

  // Insert new MBBs.
  MF->insert(++MBB.getIterator(), LoopHeadMBB);
  MF->insert(++LoopHeadMBB->getIterator(), LoopIfBodyMBB);
  MF->insert(++LoopIfBodyMBB->getIterator(), LoopTailMBB);
  MF->insert(++LoopTailMBB->getIterator(), DoneMBB);

  // Set up successors and transfer remaining instructions to DoneMBB.
  LoopHeadMBB->addSuccessor(LoopIfBodyMBB);
  LoopHeadMBB->addSuccessor(LoopTailMBB);
  LoopIfBodyMBB->addSuccessor(LoopTailMBB);
  LoopTailMBB->addSuccessor(LoopHeadMBB);
  LoopTailMBB->addSuccessor(DoneMBB);
  DoneMBB->splice(DoneMBB->end(), &MBB, MI, MBB.end());
  DoneMBB->transferSuccessors(&MBB);
  MBB.addSuccessor(LoopHeadMBB);

  Register DestReg = MI.getOperand(0).getReg();
  Register Scratch1Reg = MI.getOperand(1).getReg();
  Register Scratch2Reg = MI.getOperand(2).getReg();
  Register AddrReg = MI.getOperand(3).getReg();
  Register IncrReg = MI.getOperand(4).getReg();
  Register MaskReg = MI.getOperand(5).getReg();
  bool IsSigned = BinOp == AtomicRMWInst::Min || BinOp == AtomicRMWInst::Max;
  AtomicOrdering Ordering =
      static_cast<AtomicOrdering>(MI.getOperand(IsSigned ? 7 : 6).getImm());

  //
  // .loophead:
  //   lr.w destreg, (alignedaddr)
  //   and scratch2, destreg, mask
  //   mv scratch1, destreg
  //   [sext scratch2 if signed min/max]
  //   ifnochangeneeded scratch2, incr, .looptail
  BuildMI(LoopHeadMBB, DL, TII->get(getLRForRMW32(Ordering)), DestReg)
      .addReg(AddrReg);
  BuildMI(LoopHeadMBB, DL, TII->get(TinyRISCV::AND), Scratch2Reg)
      .addReg(DestReg)
      .addReg(MaskReg);
  BuildMI(LoopHeadMBB, DL, TII->get(TinyRISCV::ADDI), Scratch1Reg)
      .addReg(DestReg)
      .addImm(0);

  switch (BinOp) {
  default:
    llvm_unreachable("Unexpected AtomicRMW BinOp");
  case AtomicRMWInst::Max: {
    insertSext(TII, DL, LoopHeadMBB, Scratch2Reg, MI.getOperand(6).getReg());
    BuildMI(LoopHeadMBB, DL, TII->get(TinyRISCV::BGE))
        .addReg(Scratch2Reg)
        .addReg(IncrReg)
        .addMBB(LoopTailMBB);
    break;
  }
  case AtomicRMWInst::Min: {
    insertSext(TII, DL, LoopHeadMBB, Scratch2Reg, MI.getOperand(6).getReg());
    BuildMI(LoopHeadMBB, DL, TII->get(TinyRISCV::BGE))
        .addReg(IncrReg)
        .addReg(Scratch2Reg)
        .addMBB(LoopTailMBB);
    break;
  }
  case AtomicRMWInst::UMax:
    BuildMI(LoopHeadMBB, DL, TII->get(TinyRISCV::BGEU))
        .addReg(Scratch2Reg)
        .addReg(IncrReg)
        .addMBB(LoopTailMBB);
    break;
  case AtomicRMWInst::UMin:
    BuildMI(LoopHeadMBB, DL, TII->get(TinyRISCV::BGEU))
        .addReg(IncrReg)
        .addReg(Scratch2Reg)
        .addMBB(LoopTailMBB);
    break;
  }

  // .loopifbody:
  //   xor scratch1, destreg, incr
  //   and scratch1, scratch1, mask
  //   xor scratch1, destreg, scratch1
  insertMaskedMerge(TII, DL, LoopIfBodyMBB, Scratch1Reg, DestReg, IncrReg,
                    MaskReg, Scratch1Reg);

  // .looptail:
  //   sc.w scratch1, scratch1, (addr)
  //   bnez scratch1, loop
  BuildMI(LoopTailMBB, DL, TII->get(getSCForRMW32(Ordering)), Scratch1Reg)
      .addReg(AddrReg)
      .addReg(Scratch1Reg);
  BuildMI(LoopTailMBB, DL, TII->get(TinyRISCV::BNE))
      .addReg(Scratch1Reg)
      .addReg(TinyRISCV::X0)
      .addMBB(LoopHeadMBB);

  NextMBBI = MBB.end();
  MI.eraseFromParent();

  LivePhysRegs LiveRegs;
  computeAndAddLiveIns(LiveRegs, *LoopHeadMBB);
  computeAndAddLiveIns(LiveRegs, *LoopIfBodyMBB);
  computeAndAddLiveIns(LiveRegs, *LoopTailMBB);
  computeAndAddLiveIns(LiveRegs, *DoneMBB);

  return true;
}

bool TinyRISCVExpandPseudo::expandAtomicCmpXchg(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI, bool IsMasked,
    int Width, MachineBasicBlock::iterator &NextMBBI) {
  MachineInstr &MI = *MBBI;
  DebugLoc DL = MI.getDebugLoc();
  MachineFunction *MF = MBB.getParent();
  auto LoopHeadMBB = MF->CreateMachineBasicBlock(MBB.getBasicBlock());
  auto LoopTailMBB = MF->CreateMachineBasicBlock(MBB.getBasicBlock());
  auto DoneMBB = MF->CreateMachineBasicBlock(MBB.getBasicBlock());

  // Insert new MBBs.
  MF->insert(++MBB.getIterator(), LoopHeadMBB);
  MF->insert(++LoopHeadMBB->getIterator(), LoopTailMBB);
  MF->insert(++LoopTailMBB->getIterator(), DoneMBB);

  // Set up successors and transfer remaining instructions to DoneMBB.
  LoopHeadMBB->addSuccessor(LoopTailMBB);
  LoopHeadMBB->addSuccessor(DoneMBB);
  LoopTailMBB->addSuccessor(DoneMBB);
  LoopTailMBB->addSuccessor(LoopHeadMBB);
  DoneMBB->splice(DoneMBB->end(), &MBB, MI, MBB.end());
  DoneMBB->transferSuccessors(&MBB);
  MBB.addSuccessor(LoopHeadMBB);

  Register DestReg = MI.getOperand(0).getReg();
  Register ScratchReg = MI.getOperand(1).getReg();
  Register AddrReg = MI.getOperand(2).getReg();
  Register CmpValReg = MI.getOperand(3).getReg();
  Register NewValReg = MI.getOperand(4).getReg();
  AtomicOrdering Ordering =
      static_cast<AtomicOrdering>(MI.getOperand(IsMasked ? 6 : 5).getImm());

  if (!IsMasked) {
    // .loophead:
    //   lr.[w|d] dest, (addr)
    //   bne dest, cmpval, done
    BuildMI(LoopHeadMBB, DL, TII->get(getLRForRMW(Ordering, Width)), DestReg)
        .addReg(AddrReg);
    BuildMI(LoopHeadMBB, DL, TII->get(TinyRISCV::BNE))
        .addReg(DestReg)
        .addReg(CmpValReg)
        .addMBB(DoneMBB);
    // .looptail:
    //   sc.[w|d] scratch, newval, (addr)
    //   bnez scratch, loophead
    BuildMI(LoopTailMBB, DL, TII->get(getSCForRMW(Ordering, Width)), ScratchReg)
        .addReg(AddrReg)
        .addReg(NewValReg);
    BuildMI(LoopTailMBB, DL, TII->get(TinyRISCV::BNE))
        .addReg(ScratchReg)
        .addReg(TinyRISCV::X0)
        .addMBB(LoopHeadMBB);
  } else {
    // .loophead:
    //   lr.w dest, (addr)
    //   and scratch, dest, mask
    //   bne scratch, cmpval, done
    Register MaskReg = MI.getOperand(5).getReg();
    BuildMI(LoopHeadMBB, DL, TII->get(getLRForRMW(Ordering, Width)), DestReg)
        .addReg(AddrReg);
    BuildMI(LoopHeadMBB, DL, TII->get(TinyRISCV::AND), ScratchReg)
        .addReg(DestReg)
        .addReg(MaskReg);
    BuildMI(LoopHeadMBB, DL, TII->get(TinyRISCV::BNE))
        .addReg(ScratchReg)
        .addReg(CmpValReg)
        .addMBB(DoneMBB);

    // .looptail:
    //   xor scratch, dest, newval
    //   and scratch, scratch, mask
    //   xor scratch, dest, scratch
    //   sc.w scratch, scratch, (adrr)
    //   bnez scratch, loophead
    insertMaskedMerge(TII, DL, LoopTailMBB, ScratchReg, DestReg, NewValReg,
                      MaskReg, ScratchReg);
    BuildMI(LoopTailMBB, DL, TII->get(getSCForRMW(Ordering, Width)), ScratchReg)
        .addReg(AddrReg)
        .addReg(ScratchReg);
    BuildMI(LoopTailMBB, DL, TII->get(TinyRISCV::BNE))
        .addReg(ScratchReg)
        .addReg(TinyRISCV::X0)
        .addMBB(LoopHeadMBB);
  }

  NextMBBI = MBB.end();
  MI.eraseFromParent();

  LivePhysRegs LiveRegs;
  computeAndAddLiveIns(LiveRegs, *LoopHeadMBB);
  computeAndAddLiveIns(LiveRegs, *LoopTailMBB);
  computeAndAddLiveIns(LiveRegs, *DoneMBB);

  return true;
}

bool TinyRISCVExpandPseudo::expandAuipcInstPair(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
    MachineBasicBlock::iterator &NextMBBI, unsigned FlagsHi,
    unsigned SecondOpcode) {
  MachineFunction *MF = MBB.getParent();
  MachineInstr &MI = *MBBI;
  DebugLoc DL = MI.getDebugLoc();

  Register DestReg = MI.getOperand(0).getReg();
  const MachineOperand &Symbol = MI.getOperand(1);

  MachineBasicBlock *NewMBB = MF->CreateMachineBasicBlock(MBB.getBasicBlock());

  // Tell AsmPrinter that we unconditionally want the symbol of this label to be
  // emitted.
  NewMBB->setLabelMustBeEmitted();

  MF->insert(++MBB.getIterator(), NewMBB);

  BuildMI(NewMBB, DL, TII->get(TinyRISCV::AUIPC), DestReg)
      .addDisp(Symbol, 0, FlagsHi);
  BuildMI(NewMBB, DL, TII->get(SecondOpcode), DestReg)
      .addReg(DestReg)
      .addMBB(NewMBB, TinyRISCVII::MO_PCREL_LO);

  // Move all the rest of the instructions to NewMBB.
  NewMBB->splice(NewMBB->end(), &MBB, std::next(MBBI), MBB.end());
  // Update machine-CFG edges.
  NewMBB->transferSuccessorsAndUpdatePHIs(&MBB);
  // Make the original basic block fall-through to the new.
  MBB.addSuccessor(NewMBB);

  // Make sure live-ins are correctly attached to this new basic block.
  LivePhysRegs LiveRegs;
  computeAndAddLiveIns(LiveRegs, *NewMBB);

  NextMBBI = MBB.end();
  MI.eraseFromParent();
  return true;
}

bool TinyRISCVExpandPseudo::expandLoadLocalAddress(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
    MachineBasicBlock::iterator &NextMBBI) {
  return expandAuipcInstPair(MBB, MBBI, NextMBBI, TinyRISCVII::MO_PCREL_HI,
                             TinyRISCV::ADDI);
}

bool TinyRISCVExpandPseudo::expandLoadAddress(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
    MachineBasicBlock::iterator &NextMBBI) {
  MachineFunction *MF = MBB.getParent();

  unsigned SecondOpcode;
  unsigned FlagsHi;
  if (MF->getTarget().isPositionIndependent()) {
    const auto &STI = MF->getSubtarget<TinyRISCVSubtarget>();
    SecondOpcode = TinyRISCV::LW;
    FlagsHi = TinyRISCVII::MO_GOT_HI;
  } else {
    SecondOpcode = TinyRISCV::ADDI;
    FlagsHi = TinyRISCVII::MO_PCREL_HI;
  }
  return expandAuipcInstPair(MBB, MBBI, NextMBBI, FlagsHi, SecondOpcode);
}

bool TinyRISCVExpandPseudo::expandLoadTLSIEAddress(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
    MachineBasicBlock::iterator &NextMBBI) {
  MachineFunction *MF = MBB.getParent();

  const auto &STI = MF->getSubtarget<TinyRISCVSubtarget>();
  unsigned SecondOpcode = TinyRISCV::LW;
  return expandAuipcInstPair(MBB, MBBI, NextMBBI, TinyRISCVII::MO_TLS_GOT_HI,
                             SecondOpcode);
}

bool TinyRISCVExpandPseudo::expandLoadTLSGDAddress(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
    MachineBasicBlock::iterator &NextMBBI) {
  return expandAuipcInstPair(MBB, MBBI, NextMBBI, TinyRISCVII::MO_TLS_GD_HI,
                             TinyRISCV::ADDI);
}

} // end of anonymous namespace

INITIALIZE_PASS(TinyRISCVExpandPseudo, "tinyriscv-expand-pseudo",
                TinyRISCV_EXPAND_PSEUDO_NAME, false, false)
namespace llvm {

FunctionPass *createTinyRISCVExpandPseudoPass() { return new TinyRISCVExpandPseudo(); }

} // end of namespace llvm
