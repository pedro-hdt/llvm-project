//===-- TinyRISCVTargetStreamer.cpp - TinyRISCV Target Streamer Methods -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides TinyRISCV specific target streamer methods.
//
//===----------------------------------------------------------------------===//

#include "TinyRISCVTargetStreamer.h"
#include "llvm/Support/FormattedStream.h"

using namespace llvm;

TinyRISCVTargetStreamer::TinyRISCVTargetStreamer(MCStreamer &S) : MCTargetStreamer(S) {}

// This part is for ascii assembly output
TinyRISCVTargetAsmStreamer::TinyRISCVTargetAsmStreamer(MCStreamer &S,
                                               formatted_raw_ostream &OS)
    : TinyRISCVTargetStreamer(S), OS(OS) {}

void TinyRISCVTargetAsmStreamer::emitDirectiveOptionPush() {
  OS << "\t.option\tpush\n";
}

void TinyRISCVTargetAsmStreamer::emitDirectiveOptionPop() {
  OS << "\t.option\tpop\n";
}

void TinyRISCVTargetAsmStreamer::emitDirectiveOptionRVC() {
  OS << "\t.option\trvc\n";
}

void TinyRISCVTargetAsmStreamer::emitDirectiveOptionNoRVC() {
  OS << "\t.option\tnorvc\n";
}

void TinyRISCVTargetAsmStreamer::emitDirectiveOptionRelax() {
  OS << "\t.option\trelax\n";
}

void TinyRISCVTargetAsmStreamer::emitDirectiveOptionNoRelax() {
  OS << "\t.option\tnorelax\n";
}
