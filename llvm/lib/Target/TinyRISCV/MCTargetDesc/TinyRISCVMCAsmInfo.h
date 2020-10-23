//
// Created by pedro-teixeira on 23/09/2020.
//

#ifndef LLVM_LIB_TARGET_TinyRISCV_TinyRISCVTARGETASMINFO_H
#define LLVM_LIB_TARGET_TinyRISCV_TinyRISCVTARGETASMINFO_H

#include "llvm/MC/MCAsmInfoELF.h"

namespace llvm {
class Triple;

class TinyRISCVMCAsmInfo : public MCAsmInfoELF {
  void anchor() override;

public:
  explicit TinyRISCVMCAsmInfo(const Triple &TargetTriple);
};

} // namespace llvm
#endif // LLVM_LIB_TARGET_TinyRISCV_TinyRISCVTARGETASMINFO_H