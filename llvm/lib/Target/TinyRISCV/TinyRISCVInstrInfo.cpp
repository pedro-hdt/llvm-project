//
// Created by pedro-teixeira on 28/09/2020.
//

#include "TinyRISCVInstrInfo.h"

using namespace llvm;

#define GET_INSTRINFO_CTOR_DTOR
#include "TinyRISCVGenInstrInfo.inc"

TinyRISCVInstrInfo::TinyRISCVInstrInfo(TinyRISCVSubtarget &ST)
    : TinyRISCVGenInstrInfo(),
      Subtarget(ST) {}