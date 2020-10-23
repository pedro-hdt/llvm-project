//
// Created by pedro-teixeira on 30/09/2020.
//

#include "TinyRISCVMCExpr.h"
#include "TinyRISCV.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCAsmLayout.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbolELF.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

#define DEBUG_TYPE "tinyriscvmcexpr"

const TinyRISCVMCExpr* TinyRISCVMCExpr::create(const MCExpr *Expr, VariantKind Kind,
                                       MCContext &Ctx) {
  return new (Ctx) TinyRISCVMCExpr(Expr, Kind);
}

void TinyRISCVMCExpr::printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const {
  OS << '%' << getVariantKindName(getKind()) << '(';
  Expr->print(OS, MAI);
  OS << ')';
}

TinyRISCVMCExpr::VariantKind TinyRISCVMCExpr::getVariantKindForName(StringRef name) {
  return StringSwitch<TinyRISCVMCExpr::VariantKind>(name)
      .Case("lo", VK_TinyRISCV_LO)
      .Case("hi", VK_TinyRISCV_HI)
      .Default(VK_TinyRISCV_Invalid);
}

StringRef TinyRISCVMCExpr::getVariantKindName(VariantKind Kind) {
  switch (Kind) {
  default:
    llvm_unreachable("Invalid ELF symbol kind");
  case VK_TinyRISCV_LO:
    return "lo";
  case VK_TinyRISCV_HI:
    return "hi";
  }
}
bool TinyRISCVMCExpr::evaluateAsRelocatableImpl(MCValue &Res,
                                          const MCAsmLayout *Layout,
                                          const MCFixup *Fixup) const {
  if (!getSubExpr()->evaluateAsRelocatable(Res, Layout, Fixup))
    return false;

  // Some custom fixup types are not valid with symbol difference expressions
  if (Res.getSymA() && Res.getSymB()) {
    switch (getKind()) {
    default:
      return true;
    case VK_TinyRISCV_LO:
    case VK_TinyRISCV_HI:
      return false;
    }
  }

  return true;
}

void TinyRISCVMCExpr::visitUsedExpr(MCStreamer &Streamer) const {
  Streamer.visitUsedExpr(*getSubExpr());
}

void TinyRISCVMCExpr::fixELFSymbolsInTLSFixups(MCAssembler &Asm) const {
  return;
}
