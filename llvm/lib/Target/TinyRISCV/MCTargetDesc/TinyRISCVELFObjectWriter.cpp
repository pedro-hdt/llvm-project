//===-- TinyRISCVELFObjectWriter.cpp - TinyRISCV ELF Writer -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/TinyRISCVFixupKinds.h"
#include "MCTargetDesc/TinyRISCVMCExpr.h"
#include "MCTargetDesc/TinyRISCVMCTargetDesc.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

namespace {
class TinyRISCVELFObjectWriter : public MCELFObjectTargetWriter {
public:
  TinyRISCVELFObjectWriter(uint8_t OSABI, bool Is64Bit);

  ~TinyRISCVELFObjectWriter() override;

  // Return true if the given relocation must be with a symbol rather than
  // section plus offset.
  bool needsRelocateWithSymbol(const MCSymbol &Sym,
                               unsigned Type) const override {
    // TODO: this is very conservative, update once RISC-V psABI requirements
    //       are clarified.
    return true;
  }

protected:
  unsigned getRelocType(MCContext &Ctx, const MCValue &Target,
                        const MCFixup &Fixup, bool IsPCRel) const override;
};
}

TinyRISCVELFObjectWriter::TinyRISCVELFObjectWriter(uint8_t OSABI, bool Is64Bit)
    : MCELFObjectTargetWriter(Is64Bit, OSABI, ELF::EM_TINYRISCV,
                              /*HasRelocationAddend*/ true) {}

TinyRISCVELFObjectWriter::~TinyRISCVELFObjectWriter() {}

unsigned TinyRISCVELFObjectWriter::getRelocType(MCContext &Ctx,
                                            const MCValue &Target,
                                            const MCFixup &Fixup,
                                            bool IsPCRel) const {
  const MCExpr *Expr = Fixup.getValue();
  // Determine the type of the relocation
  unsigned Kind = Fixup.getTargetKind();
  Ctx.reportError(Fixup.getLoc(), "Unsupported relocation type");
  return ELF::R_RISCV_NONE;
}

std::unique_ptr<MCObjectTargetWriter>
llvm::createTinyRISCVELFObjectWriter(uint8_t OSABI, bool Is64Bit) {
  return std::make_unique<TinyRISCVELFObjectWriter>(OSABI, Is64Bit);
}
