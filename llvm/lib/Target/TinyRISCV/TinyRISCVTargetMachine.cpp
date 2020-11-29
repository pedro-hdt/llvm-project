//===-- TinyRISCVTargetMachine.cpp - Define TargetMachine for TinyRISCV -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Implements the info about TinyRISCV target spec.
//
//===----------------------------------------------------------------------===//

#include "TinyRISCVTargetMachine.h"
#include "TinyRISCV.h"
#include "TinyRISCVTargetObjectFile.h"
#include "TinyRISCVTargetTransformInfo.h"
#include "TargetInfo/TinyRISCVTargetInfo.h"
#include "Utils/TinyRISCVBaseInfo.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/CodeGen/GlobalISel/IRTranslator.h"
#include "llvm/CodeGen/GlobalISel/InstructionSelect.h"
#include "llvm/CodeGen/GlobalISel/Legalizer.h"
#include "llvm/CodeGen/GlobalISel/RegBankSelect.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Target/TargetOptions.h"
using namespace llvm;

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeTinyRISCVTarget() {
  RegisterTargetMachine<TinyRISCVTargetMachine> X(getTheTinyRISCVTarget());
  auto PR = PassRegistry::getPassRegistry();
  initializeGlobalISel(*PR);
  initializeTinyRISCVExpandPseudoPass(*PR);
}

static StringRef computeDataLayout(const Triple &TT) {
  return "e-m:e-p:32:32-i64:64-n32-S128";
}

static Reloc::Model getEffectiveRelocModel(const Triple &TT,
                                           Optional<Reloc::Model> RM) {
  if (!RM.hasValue())
    return Reloc::Static;
  return *RM;
}

TinyRISCVTargetMachine::TinyRISCVTargetMachine(const Target &T, const Triple &TT,
                                       StringRef CPU, StringRef FS,
                                       const TargetOptions &Options,
                                       Optional<Reloc::Model> RM,
                                       Optional<CodeModel::Model> CM,
                                       CodeGenOpt::Level OL, bool JIT)
    : LLVMTargetMachine(T, computeDataLayout(TT), TT, CPU, FS, Options,
                        getEffectiveRelocModel(TT, RM),
                        getEffectiveCodeModel(CM, CodeModel::Small), OL),
      TLOF(std::make_unique<TinyRISCVELFTargetObjectFile>()) {
  initAsmInfo();

  // RISC-V supports the MachineOutliner.
  setMachineOutliner(true);
}

const TinyRISCVSubtarget *
TinyRISCVTargetMachine::getSubtargetImpl(const Function &F) const {
  Attribute CPUAttr = F.getFnAttribute("target-cpu");
  Attribute FSAttr = F.getFnAttribute("target-features");

  std::string CPU = !CPUAttr.hasAttribute(Attribute::None)
                        ? CPUAttr.getValueAsString().str()
                        : TargetCPU;
  std::string FS = !FSAttr.hasAttribute(Attribute::None)
                       ? FSAttr.getValueAsString().str()
                       : TargetFS;
  std::string Key = CPU + FS;
  auto &I = SubtargetMap[Key];
  if (!I) {
    // This needs to be done before we create a new subtarget since any
    // creation will depend on the TM and the code generation flags on the
    // function that reside in TargetOptions.
    resetTargetOptions(F);
    auto ABIName = Options.MCOptions.getABIName();
    if (const MDString *ModuleTargetABI = dyn_cast_or_null<MDString>(
            F.getParent()->getModuleFlag("target-abi"))) {
      auto TargetABI = TinyRISCVABI::getTargetABI(ABIName);
      if (TargetABI != TinyRISCVABI::ABI_Unknown &&
          ModuleTargetABI->getString() != ABIName) {
        report_fatal_error("-target-abi option != target-abi module flag");
      }
      ABIName = ModuleTargetABI->getString();
    }
    I = std::make_unique<TinyRISCVSubtarget>(TargetTriple, CPU, FS, ABIName, *this);
  }
  return I.get();
}

TargetTransformInfo
TinyRISCVTargetMachine::getTargetTransformInfo(const Function &F) {
  return TargetTransformInfo(TinyRISCVTTIImpl(this, F));
}

namespace {
class TinyRISCVPassConfig : public TargetPassConfig {
public:
  TinyRISCVPassConfig(TinyRISCVTargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}

  TinyRISCVTargetMachine &getTinyRISCVTargetMachine() const {
    return getTM<TinyRISCVTargetMachine>();
  }

  void addIRPasses() override;
  bool addInstSelector() override;
  bool addIRTranslator() override;
  bool addLegalizeMachineIR() override;
  bool addRegBankSelect() override;
  bool addGlobalInstructionSelect() override;
  void addPreEmitPass() override;
  void addPreEmitPass2() override;
  void addPreRegAlloc() override;
};
}

TargetPassConfig *TinyRISCVTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new TinyRISCVPassConfig(*this, PM);
}

void TinyRISCVPassConfig::addIRPasses() {
  addPass(createAtomicExpandPass());
  TargetPassConfig::addIRPasses();
}

bool TinyRISCVPassConfig::addInstSelector() {
  addPass(createTinyRISCVISelDag(getTinyRISCVTargetMachine()));

  return false;
}

bool TinyRISCVPassConfig::addIRTranslator() {
  addPass(new IRTranslator());
  return false;
}

bool TinyRISCVPassConfig::addLegalizeMachineIR() {
  addPass(new Legalizer());
  return false;
}

bool TinyRISCVPassConfig::addRegBankSelect() {
  addPass(new RegBankSelect());
  return false;
}

bool TinyRISCVPassConfig::addGlobalInstructionSelect() {
  addPass(new InstructionSelect());
  return false;
}

void TinyRISCVPassConfig::addPreEmitPass() { addPass(&BranchRelaxationPassID); }

void TinyRISCVPassConfig::addPreEmitPass2() {
  // Schedule the expansion of AMOs at the last possible moment, avoiding the
  // possibility for other passes to break the requirements for forward
  // progress in the LR/SC block.
  addPass(createTinyRISCVExpandPseudoPass());
}

void TinyRISCVPassConfig::addPreRegAlloc() {
  addPass(createTinyRISCVMergeBaseOffsetOptPass());
}
