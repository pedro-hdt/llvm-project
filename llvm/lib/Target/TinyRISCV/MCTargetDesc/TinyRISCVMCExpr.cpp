//===-- TinyRISCVMCExpr.cpp - TinyRISCV specific MC expression classes ------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the implementation of the assembly expression modifiers
// accepted by the TinyRISCV architecture (e.g. ":lo12:", ":gottprel_g1:", ...).
//
//===----------------------------------------------------------------------===//

#include "TinyRISCVMCExpr.h"
#include "MCTargetDesc/TinyRISCVAsmBackend.h"
#include "TinyRISCV.h"
#include "TinyRISCVFixupKinds.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCAsmLayout.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbolELF.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

#define DEBUG_TYPE "riscvmcexpr"

const TinyRISCVMCExpr *TinyRISCVMCExpr::create(const MCExpr *Expr, VariantKind Kind,
                                       MCContext &Ctx) {
  return new (Ctx) TinyRISCVMCExpr(Expr, Kind);
}

void TinyRISCVMCExpr::printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const {
  VariantKind Kind = getKind();
  bool HasVariant = ((Kind != VK_TinyRISCV_None) && (Kind != VK_TinyRISCV_CALL) &&
                     (Kind != VK_TinyRISCV_CALL_PLT));

  if (HasVariant)
    OS << '%' << getVariantKindName(getKind()) << '(';
  Expr->print(OS, MAI);
  if (Kind == VK_TinyRISCV_CALL_PLT)
    OS << "@plt";
  if (HasVariant)
    OS << ')';
}

const MCFixup *TinyRISCVMCExpr::getPCRelHiFixup(const MCFragment **DFOut) const {
  MCValue AUIPCLoc;
  if (!getSubExpr()->evaluateAsRelocatable(AUIPCLoc, nullptr, nullptr))
    return nullptr;

  const MCSymbolRefExpr *AUIPCSRE = AUIPCLoc.getSymA();
  if (!AUIPCSRE)
    return nullptr;

  const MCSymbol *AUIPCSymbol = &AUIPCSRE->getSymbol();
  const auto *DF = dyn_cast_or_null<MCDataFragment>(AUIPCSymbol->getFragment());

  if (!DF)
    return nullptr;

  uint64_t Offset = AUIPCSymbol->getOffset();
  if (DF->getContents().size() == Offset) {
    DF = dyn_cast_or_null<MCDataFragment>(DF->getNextNode());
    if (!DF)
      return nullptr;
    Offset = 0;
  }

  for (const MCFixup &F : DF->getFixups()) {
    if (F.getOffset() != Offset)
      continue;

    switch ((unsigned)F.getKind()) {
    default:
      continue;
    case TinyRISCV::fixup_riscv_got_hi20:
    case TinyRISCV::fixup_riscv_tls_got_hi20:
    case TinyRISCV::fixup_riscv_tls_gd_hi20:
    case TinyRISCV::fixup_riscv_pcrel_hi20:
      if (DFOut)
        *DFOut = DF;
      return &F;
    }
  }

  return nullptr;
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
    case VK_TinyRISCV_PCREL_LO:
    case VK_TinyRISCV_PCREL_HI:
    case VK_TinyRISCV_GOT_HI:
    case VK_TinyRISCV_TPREL_LO:
    case VK_TinyRISCV_TPREL_HI:
    case VK_TinyRISCV_TPREL_ADD:
    case VK_TinyRISCV_TLS_GOT_HI:
    case VK_TinyRISCV_TLS_GD_HI:
      return false;
    }
  }

  return true;
}

void TinyRISCVMCExpr::visitUsedExpr(MCStreamer &Streamer) const {
  Streamer.visitUsedExpr(*getSubExpr());
}

TinyRISCVMCExpr::VariantKind TinyRISCVMCExpr::getVariantKindForName(StringRef name) {
  return StringSwitch<TinyRISCVMCExpr::VariantKind>(name)
      .Case("lo", VK_TinyRISCV_LO)
      .Case("hi", VK_TinyRISCV_HI)
      .Case("pcrel_lo", VK_TinyRISCV_PCREL_LO)
      .Case("pcrel_hi", VK_TinyRISCV_PCREL_HI)
      .Case("got_pcrel_hi", VK_TinyRISCV_GOT_HI)
      .Case("tprel_lo", VK_TinyRISCV_TPREL_LO)
      .Case("tprel_hi", VK_TinyRISCV_TPREL_HI)
      .Case("tprel_add", VK_TinyRISCV_TPREL_ADD)
      .Case("tls_ie_pcrel_hi", VK_TinyRISCV_TLS_GOT_HI)
      .Case("tls_gd_pcrel_hi", VK_TinyRISCV_TLS_GD_HI)
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
  case VK_TinyRISCV_PCREL_LO:
    return "pcrel_lo";
  case VK_TinyRISCV_PCREL_HI:
    return "pcrel_hi";
  case VK_TinyRISCV_GOT_HI:
    return "got_pcrel_hi";
  case VK_TinyRISCV_TPREL_LO:
    return "tprel_lo";
  case VK_TinyRISCV_TPREL_HI:
    return "tprel_hi";
  case VK_TinyRISCV_TPREL_ADD:
    return "tprel_add";
  case VK_TinyRISCV_TLS_GOT_HI:
    return "tls_ie_pcrel_hi";
  case VK_TinyRISCV_TLS_GD_HI:
    return "tls_gd_pcrel_hi";
  }
}

static void fixELFSymbolsInTLSFixupsImpl(const MCExpr *Expr, MCAssembler &Asm) {
  switch (Expr->getKind()) {
  case MCExpr::Target:
    llvm_unreachable("Can't handle nested target expression");
    break;
  case MCExpr::Constant:
    break;

  case MCExpr::Binary: {
    const MCBinaryExpr *BE = cast<MCBinaryExpr>(Expr);
    fixELFSymbolsInTLSFixupsImpl(BE->getLHS(), Asm);
    fixELFSymbolsInTLSFixupsImpl(BE->getRHS(), Asm);
    break;
  }

  case MCExpr::SymbolRef: {
    // We're known to be under a TLS fixup, so any symbol should be
    // modified. There should be only one.
    const MCSymbolRefExpr &SymRef = *cast<MCSymbolRefExpr>(Expr);
    cast<MCSymbolELF>(SymRef.getSymbol()).setType(ELF::STT_TLS);
    break;
  }

  case MCExpr::Unary:
    fixELFSymbolsInTLSFixupsImpl(cast<MCUnaryExpr>(Expr)->getSubExpr(), Asm);
    break;
  }
}

void TinyRISCVMCExpr::fixELFSymbolsInTLSFixups(MCAssembler &Asm) const {
  switch (getKind()) {
  default:
    return;
  case VK_TinyRISCV_TPREL_HI:
  case VK_TinyRISCV_TLS_GOT_HI:
  case VK_TinyRISCV_TLS_GD_HI:
    break;
  }

  fixELFSymbolsInTLSFixupsImpl(getSubExpr(), Asm);
}

bool TinyRISCVMCExpr::evaluateAsConstant(int64_t &Res) const {
  MCValue Value;

  if (Kind == VK_TinyRISCV_PCREL_HI || Kind == VK_TinyRISCV_PCREL_LO ||
      Kind == VK_TinyRISCV_GOT_HI || Kind == VK_TinyRISCV_TPREL_HI ||
      Kind == VK_TinyRISCV_TPREL_LO || Kind == VK_TinyRISCV_TPREL_ADD ||
      Kind == VK_TinyRISCV_TLS_GOT_HI || Kind == VK_TinyRISCV_TLS_GD_HI ||
      Kind == VK_TinyRISCV_CALL || Kind == VK_TinyRISCV_CALL_PLT)
    return false;

  if (!getSubExpr()->evaluateAsRelocatable(Value, nullptr, nullptr))
    return false;

  if (!Value.isAbsolute())
    return false;

  Res = evaluateAsInt64(Value.getConstant());
  return true;
}

int64_t TinyRISCVMCExpr::evaluateAsInt64(int64_t Value) const {
  switch (Kind) {
  default:
    llvm_unreachable("Invalid kind");
  case VK_TinyRISCV_LO:
    return SignExtend64<12>(Value);
  case VK_TinyRISCV_HI:
    // Add 1 if bit 11 is 1, to compensate for low 12 bits being negative.
    return ((Value + 0x800) >> 12) & 0xfffff;
  }
}
