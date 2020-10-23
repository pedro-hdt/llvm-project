//
// Created by pedro-teixeira on 04/10/2020.
//

#ifndef LLVM_LIB_TARGET_TinyRISCV_TinyRISCVTARGETOBJECTFILE_H
#define LLVM_LIB_TARGET_TinyRISCV_TinyRISCVTARGETOBJECTFILE_H

#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"

namespace llvm {
class TinyRISCVTargetMachine;

class TinyRISCVELFTargetObjectFile : public TargetLoweringObjectFileELF {
  MCSection *SmallDataSection;
  MCSection *SmallBSSSection;
  unsigned SSThreshold = 8;

public:
  void Initialize(MCContext &Ctx, const TargetMachine &TM) override;

  /// Return true if this global address should be placed into small data/bss
  /// section.
  bool isGlobalInSmallSection(const GlobalObject *GO,
                              const TargetMachine &TM) const;

  MCSection *SelectSectionForGlobal(const GlobalObject *GO, SectionKind Kind,
                                    const TargetMachine &TM) const override;

  /// Return true if this constant should be placed into small data section.
  bool isConstantInSmallSection(const DataLayout &DL, const Constant *CN) const;

  MCSection *getSectionForConstant(const DataLayout &DL, SectionKind Kind,
                                   const Constant *C,
                                   unsigned &Align) const override;

  void getModuleMetadata(Module &M) override;

  bool isInSmallSection(uint64_t Size) const;
};
} // end namespace llvm

#endif // LLVM_LIB_TARGET_TinyRISCV_TinyRISCVTARGETOBJECTFILE_H
