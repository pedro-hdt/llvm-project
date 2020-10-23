//
// Created by pedro-teixeira on 23/09/2020.
//

#include "TinyRISCVSubtarget.h"
#include "TinyRISCV.h"
#include "TinyRISCVFrameLowering.h"
#include "TinyRISCVTargetMachine.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define DEBUG_TYPE "tinyriscv-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "TinyRISCVGenSubtargetInfo.inc"

void TinyRISCVSubtarget::anchor() {}


TinyRISCVSubtarget::TinyRISCVSubtarget(const Triple &TT, StringRef CPU, StringRef FS,
                           const TargetMachine &TM)
    : TinyRISCVGenSubtargetInfo(TT, CPU, FS),
      InstrInfo(*this), RegInfo(), TLInfo(TM, *this), FrameLowering(*this) {

}