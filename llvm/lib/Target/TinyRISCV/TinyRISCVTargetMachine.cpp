//
// Created by pedro-teixeira on 23/09/2020.
//

#include "TinyRISCVTargetMachine.h"
#include "TinyRISCV.h"
#include "TinyRISCVTargetObjectFile.h"
#include "TargetInfo/TinyRISCVTargetInfo.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Analysis/TargetTransformInfo.h"
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
  // TODO initialize passes here
}

static StringRef computeDataLayout(const Triple &TT) {
  return "e-m:e-p:32:32-i32:32-n32-S32";
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
    I = std::make_unique<TinyRISCVSubtarget>(TargetTriple, CPU, FS, *this);
  }
  return I.get();
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
