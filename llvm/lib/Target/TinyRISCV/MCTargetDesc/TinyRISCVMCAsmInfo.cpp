//
// Created by pedro-teixeira on 23/09/2020.
//

#include "TinyRISCVMCAsmInfo.h"
#include "llvm/ADT/Triple.h"

using namespace llvm;

void TinyRISCVMCAsmInfo::anchor() {}

TinyRISCVMCAsmInfo::TinyRISCVMCAsmInfo(const Triple &TT) {
  SupportsDebugInformation = true;
  CodePointerSize = CalleeSaveStackSlotSize = 4;
  Data16bitsDirective = "\t.short\t";
  Data32bitsDirective = "\t.long\t";
  Data64bitsDirective = 0;
  ZeroDirective = "\t.space\t";
  CommentString = "#";
  AscizDirective = ".asciiz";

  HiddenVisibilityAttr = MCSA_Invalid;
  HiddenDeclarationVisibilityAttr = MCSA_Invalid;
  ProtectedVisibilityAttr = MCSA_Invalid;
}