//
// Created by pedro-teixeira on 20/09/2020.
//

#ifndef LLVM_LIB_TARGET_TinyRISCV_TinyRISCVFRAMELOWERING_H
#define LLVM_LIB_TARGET_TinyRISCV_TinyRISCVFRAMELOWERING_H

#include "llvm/CodeGen/TargetFrameLowering.h"

namespace llvm {
class TinyRISCVSubtarget;

class TinyRISCVFrameLowering : public TargetFrameLowering {
public:
  explicit TinyRISCVFrameLowering(const TinyRISCVSubtarget &STI)
      : TargetFrameLowering(StackGrowsDown,
                            /*StackAlignment=*/Align(16),
                            /*LocalAreaOffset=*/0),
        STI(STI) {}

  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;


  static unsigned materializeOffset(MachineFunction &MF,
                                    MachineBasicBlock &MBB,
                                    MachineBasicBlock::iterator MBBI,
                                    unsigned int Offset);
  uint64_t computeStackSize(MachineFunction &MF) const;
  bool hasFP(const MachineFunction &MF) const override { return false; };

protected:
  const TinyRISCVSubtarget &STI;
};
}

#endif // LLVM_LIB_TARGET_TinyRISCV_TinyRISCVFRAMELOWERING_H
