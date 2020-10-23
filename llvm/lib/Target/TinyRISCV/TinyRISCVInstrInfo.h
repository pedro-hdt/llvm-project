//
// Created by pedro-teixeira on 28/09/2020.
//

#ifndef LLVM_LIB_TARGET_TinyRISCV_TinyRISCVINSTRINFO_H
#define LLVM_LIB_TARGET_TinyRISCV_TinyRISCVINSTRINFO_H

#include "TinyRISCVRegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "TinyRISCVGenInstrInfo.inc"


namespace llvm {

class TinyRISCVSubtarget;

class TinyRISCVInstrInfo : public TinyRISCVGenInstrInfo {
  const TinyRISCVSubtarget& Subtarget;
public:
  explicit TinyRISCVInstrInfo(TinyRISCVSubtarget &STI);
};

}

#endif // LLVM_LIB_TARGET_TinyRISCV_TinyRISCVINSTRINFO_H
