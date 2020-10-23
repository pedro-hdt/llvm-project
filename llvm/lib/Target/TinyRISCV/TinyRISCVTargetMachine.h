//
// Created by pedro-teixeira on 23/09/2020.
//

#ifndef LLVM_LIB_TARGET_TinyRISCV_TinyRISCVTARGETMACHINE_H
#define LLVM_LIB_TARGET_TinyRISCV_TinyRISCVTARGETMACHINE_H

#include "MCTargetDesc/TinyRISCVMCTargetDesc.h"
#include "TinyRISCVSubtarget.h"
#include "llvm/CodeGen/SelectionDAGTargetInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
class TinyRISCVTargetMachine : public LLVMTargetMachine {
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  TinyRISCVSubtarget Subtarget;
  mutable StringMap<std::unique_ptr<TinyRISCVSubtarget>> SubtargetMap;

public:
  TinyRISCVTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                     StringRef FS, const TargetOptions &Options,
                     Optional<Reloc::Model> RM, Optional<CodeModel::Model> CM,
                     CodeGenOpt::Level OL, bool JIT);

  const TinyRISCVSubtarget *getSubtargetImpl() const { return &Subtarget; };
  const TinyRISCVSubtarget *getSubtargetImpl(const Function &F) const override;

  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;
  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }
};
}

#endif
