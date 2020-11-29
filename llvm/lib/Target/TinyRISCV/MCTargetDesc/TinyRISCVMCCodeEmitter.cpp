//===-- TinyRISCVMCCodeEmitter.cpp - Convert TinyRISCV code to machine code -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the TinyRISCVMCCodeEmitter class.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/TinyRISCVFixupKinds.h"
#include "MCTargetDesc/TinyRISCVMCExpr.h"
#include "MCTargetDesc/TinyRISCVMCTargetDesc.h"
#include "Utils/TinyRISCVBaseInfo.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/Register.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstBuilder.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/EndianStream.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "mccodeemitter"

STATISTIC(MCNumEmitted, "Number of MC instructions emitted");
STATISTIC(MCNumFixups, "Number of MC fixups created");

namespace {
class TinyRISCVMCCodeEmitter : public MCCodeEmitter {
  TinyRISCVMCCodeEmitter(const TinyRISCVMCCodeEmitter &) = delete;
  void operator=(const TinyRISCVMCCodeEmitter &) = delete;
  MCContext &Ctx;
  MCInstrInfo const &MCII;

public:
  TinyRISCVMCCodeEmitter(MCContext &ctx, MCInstrInfo const &MCII)
      : Ctx(ctx), MCII(MCII) {}

  ~TinyRISCVMCCodeEmitter() override {}

  void encodeInstruction(const MCInst &MI, raw_ostream &OS,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const override;

  void expandFunctionCall(const MCInst &MI, raw_ostream &OS,
                          SmallVectorImpl<MCFixup> &Fixups,
                          const MCSubtargetInfo &STI) const;

  void expandAddTPRel(const MCInst &MI, raw_ostream &OS,
                      SmallVectorImpl<MCFixup> &Fixups,
                      const MCSubtargetInfo &STI) const;

  /// TableGen'erated function for getting the binary encoding for an
  /// instruction.
  uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;

  /// Return binary encoding of operand. If the machine operand requires
  /// relocation, record the relocation and return zero.
  unsigned getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;

  unsigned getImmOpValueAsr1(const MCInst &MI, unsigned OpNo,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;

  unsigned getImmOpValue(const MCInst &MI, unsigned OpNo,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const;
};
} // end anonymous namespace

MCCodeEmitter *llvm::createTinyRISCVMCCodeEmitter(const MCInstrInfo &MCII,
                                              const MCRegisterInfo &MRI,
                                              MCContext &Ctx) {
  return new TinyRISCVMCCodeEmitter(Ctx, MCII);
}

// Expand PseudoCALL(Reg) and PseudoTAIL to AUIPC and JALR with relocation
// types. We expand PseudoCALL(Reg) and PseudoTAIL while encoding, meaning AUIPC
// and JALR won't go through TinyRISCV MC to MC compressed instruction
// transformation. This is acceptable because AUIPC has no 16-bit form and
// C_JALR have no immediate operand field.  We let linker relaxation deal with
// it. When linker relaxation enabled, AUIPC and JALR have chance relax to JAL.
// If C extension is enabled, JAL has chance relax to C_JAL.
void TinyRISCVMCCodeEmitter::expandFunctionCall(const MCInst &MI, raw_ostream &OS,
                                            SmallVectorImpl<MCFixup> &Fixups,
                                            const MCSubtargetInfo &STI) const {
  MCInst TmpInst;
  MCOperand Func;
  Register Ra;
  /*if (MI.getOpcode() == TinyRISCV::PseudoTAIL) {
    Func = MI.getOperand(0);
    Ra = TinyRISCV::X6;
  } else */if (MI.getOpcode() == TinyRISCV::PseudoCALLReg) {
    Func = MI.getOperand(1);
    Ra = MI.getOperand(0).getReg();
  } else {
    Func = MI.getOperand(0);
    Ra = TinyRISCV::X1;
  }
  uint32_t Binary;

  assert(Func.isExpr() && "Expected expression");

  const MCExpr *CallExpr = Func.getExpr();

  // Emit AUIPC Ra, Func with R_TinyRISCV_CALL relocation type.
  TmpInst = MCInstBuilder(TinyRISCV::AUIPC)
                .addReg(Ra)
                .addOperand(MCOperand::createExpr(CallExpr));
  Binary = getBinaryCodeForInstr(TmpInst, Fixups, STI);
  support::endian::write(OS, Binary, support::little);

//  if (MI.getOpcode() == TinyRISCV::PseudoTAIL)
//    // Emit JALR X0, X6, 0
//    TmpInst = MCInstBuilder(TinyRISCV::JALR).addReg(TinyRISCV::X0).addReg(Ra).addImm(0);
//  else
    // Emit JALR Ra, Ra, 0
    TmpInst = MCInstBuilder(TinyRISCV::JALR).addReg(Ra).addReg(Ra).addImm(0);
  Binary = getBinaryCodeForInstr(TmpInst, Fixups, STI);
  support::endian::write(OS, Binary, support::little);
}

// Expand PseudoAddTPRel to a simple ADD with the correct relocation.
void TinyRISCVMCCodeEmitter::expandAddTPRel(const MCInst &MI, raw_ostream &OS,
                                        SmallVectorImpl<MCFixup> &Fixups,
                                        const MCSubtargetInfo &STI) const {
  MCOperand DestReg = MI.getOperand(0);
  MCOperand SrcReg = MI.getOperand(1);
  MCOperand TPReg = MI.getOperand(2);
  assert(TPReg.isReg() && TPReg.getReg() == TinyRISCV::X4 &&
         "Expected thread pointer as second input to TP-relative add");

  MCOperand SrcSymbol = MI.getOperand(3);
  assert(SrcSymbol.isExpr() &&
         "Expected expression as third input to TP-relative add");

  const TinyRISCVMCExpr *Expr = dyn_cast<TinyRISCVMCExpr>(SrcSymbol.getExpr());
  assert(Expr && Expr->getKind() == TinyRISCVMCExpr::VK_TinyRISCV_TPREL_ADD &&
         "Expected tprel_add relocation on TP-relative symbol");

  // Emit the correct tprel_add relocation for the symbol.
  Fixups.push_back(MCFixup::create(
      0, Expr, MCFixupKind(TinyRISCV::fixup_riscv_tprel_add), MI.getLoc()));

  // Emit a normal ADD instruction with the given operands.
  MCInst TmpInst = MCInstBuilder(TinyRISCV::ADD)
                       .addOperand(DestReg)
                       .addOperand(SrcReg)
                       .addOperand(TPReg);
  uint32_t Binary = getBinaryCodeForInstr(TmpInst, Fixups, STI);
  support::endian::write(OS, Binary, support::little);
}

void TinyRISCVMCCodeEmitter::encodeInstruction(const MCInst &MI, raw_ostream &OS,
                                           SmallVectorImpl<MCFixup> &Fixups,
                                           const MCSubtargetInfo &STI) const {
  const MCInstrDesc &Desc = MCII.get(MI.getOpcode());
  // Get byte count of instruction.
  unsigned Size = Desc.getSize();

  if (MI.getOpcode() == TinyRISCV::PseudoCALLReg ||
      MI.getOpcode() == TinyRISCV::PseudoCALL) {
    expandFunctionCall(MI, OS, Fixups, STI);
    MCNumEmitted += 2;
    return;
  }

  if (MI.getOpcode() == TinyRISCV::PseudoAddTPRel) {
    expandAddTPRel(MI, OS, Fixups, STI);
    MCNumEmitted += 1;
    return;
  }

  switch (Size) {
  default:
    llvm_unreachable("Unhandled encodeInstruction length!");
  case 2: {
    uint16_t Bits = getBinaryCodeForInstr(MI, Fixups, STI);
    support::endian::write<uint16_t>(OS, Bits, support::little);
    break;
  }
  case 4: {
    uint32_t Bits = getBinaryCodeForInstr(MI, Fixups, STI);
    support::endian::write(OS, Bits, support::little);
    break;
  }
  }

  ++MCNumEmitted; // Keep track of the # of mi's emitted.
}

unsigned
TinyRISCVMCCodeEmitter::getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                                      SmallVectorImpl<MCFixup> &Fixups,
                                      const MCSubtargetInfo &STI) const {

  if (MO.isReg())
    return Ctx.getRegisterInfo()->getEncodingValue(MO.getReg());

  if (MO.isImm())
    return static_cast<unsigned>(MO.getImm());

  llvm_unreachable("Unhandled expression!");
  return 0;
}

unsigned
TinyRISCVMCCodeEmitter::getImmOpValueAsr1(const MCInst &MI, unsigned OpNo,
                                      SmallVectorImpl<MCFixup> &Fixups,
                                      const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(OpNo);

  if (MO.isImm()) {
    unsigned Res = MO.getImm();
    assert((Res & 1) == 0 && "LSB is non-zero");
    return Res >> 1;
  }

  return getImmOpValue(MI, OpNo, Fixups, STI);
}

unsigned TinyRISCVMCCodeEmitter::getImmOpValue(const MCInst &MI, unsigned OpNo,
                                           SmallVectorImpl<MCFixup> &Fixups,
                                           const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(OpNo);

  MCInstrDesc const &Desc = MCII.get(MI.getOpcode());
  unsigned MIFrm = Desc.TSFlags & TinyRISCVII::InstFormatMask;

  // If the destination is an immediate, there is nothing to do.
  if (MO.isImm())
    return MO.getImm();

  assert(MO.isExpr() &&
         "getImmOpValue expects only expressions or immediates");
  const MCExpr *Expr = MO.getExpr();
  MCExpr::ExprKind Kind = Expr->getKind();
  TinyRISCV::Fixups FixupKind = TinyRISCV::fixup_riscv_invalid;
  bool RelaxCandidate = false;
  if (Kind == MCExpr::Target) {
    const TinyRISCVMCExpr *RVExpr = cast<TinyRISCVMCExpr>(Expr);

    switch (RVExpr->getKind()) {
    case TinyRISCVMCExpr::VK_TinyRISCV_None:
    case TinyRISCVMCExpr::VK_TinyRISCV_Invalid:
    case TinyRISCVMCExpr::VK_TinyRISCV_32_PCREL:
      llvm_unreachable("Unhandled fixup kind!");
    case TinyRISCVMCExpr::VK_TinyRISCV_TPREL_ADD:
      // tprel_add is only used to indicate that a relocation should be emitted
      // for an add instruction used in TP-relative addressing. It should not be
      // expanded as if representing an actual instruction operand and so to
      // encounter it here is an error.
      llvm_unreachable(
          "VK_TinyRISCV_TPREL_ADD should not represent an instruction operand");
    case TinyRISCVMCExpr::VK_TinyRISCV_LO:
      if (MIFrm == TinyRISCVII::InstFormatI)
        FixupKind = TinyRISCV::fixup_riscv_lo12_i;
      else if (MIFrm == TinyRISCVII::InstFormatS)
        FixupKind = TinyRISCV::fixup_riscv_lo12_s;
      else
        llvm_unreachable("VK_TinyRISCV_LO used with unexpected instruction format");
      RelaxCandidate = true;
      break;
    case TinyRISCVMCExpr::VK_TinyRISCV_HI:
      FixupKind = TinyRISCV::fixup_riscv_hi20;
      RelaxCandidate = true;
      break;
    case TinyRISCVMCExpr::VK_TinyRISCV_PCREL_LO:
      if (MIFrm == TinyRISCVII::InstFormatI)
        FixupKind = TinyRISCV::fixup_riscv_pcrel_lo12_i;
      else if (MIFrm == TinyRISCVII::InstFormatS)
        FixupKind = TinyRISCV::fixup_riscv_pcrel_lo12_s;
      else
        llvm_unreachable(
            "VK_TinyRISCV_PCREL_LO used with unexpected instruction format");
      RelaxCandidate = true;
      break;
    case TinyRISCVMCExpr::VK_TinyRISCV_PCREL_HI:
      FixupKind = TinyRISCV::fixup_riscv_pcrel_hi20;
      RelaxCandidate = true;
      break;
    case TinyRISCVMCExpr::VK_TinyRISCV_GOT_HI:
      FixupKind = TinyRISCV::fixup_riscv_got_hi20;
      break;
    case TinyRISCVMCExpr::VK_TinyRISCV_TPREL_LO:
      if (MIFrm == TinyRISCVII::InstFormatI)
        FixupKind = TinyRISCV::fixup_riscv_tprel_lo12_i;
      else if (MIFrm == TinyRISCVII::InstFormatS)
        FixupKind = TinyRISCV::fixup_riscv_tprel_lo12_s;
      else
        llvm_unreachable(
            "VK_TinyRISCV_TPREL_LO used with unexpected instruction format");
      RelaxCandidate = true;
      break;
    case TinyRISCVMCExpr::VK_TinyRISCV_TPREL_HI:
      FixupKind = TinyRISCV::fixup_riscv_tprel_hi20;
      RelaxCandidate = true;
      break;
    case TinyRISCVMCExpr::VK_TinyRISCV_TLS_GOT_HI:
      FixupKind = TinyRISCV::fixup_riscv_tls_got_hi20;
      break;
    case TinyRISCVMCExpr::VK_TinyRISCV_TLS_GD_HI:
      FixupKind = TinyRISCV::fixup_riscv_tls_gd_hi20;
      break;
    case TinyRISCVMCExpr::VK_TinyRISCV_CALL:
      FixupKind = TinyRISCV::fixup_riscv_call;
      RelaxCandidate = true;
      break;
    case TinyRISCVMCExpr::VK_TinyRISCV_CALL_PLT:
      FixupKind = TinyRISCV::fixup_riscv_call_plt;
      RelaxCandidate = true;
      break;
    }
  } else if (Kind == MCExpr::SymbolRef &&
             cast<MCSymbolRefExpr>(Expr)->getKind() == MCSymbolRefExpr::VK_None) {
    if (Desc.getOpcode() == TinyRISCV::JAL) {
      FixupKind = TinyRISCV::fixup_riscv_jal;
    } else if (MIFrm == TinyRISCVII::InstFormatB) {
      FixupKind = TinyRISCV::fixup_riscv_branch;
    } else if (MIFrm == TinyRISCVII::InstFormatCJ) {
      FixupKind = TinyRISCV::fixup_riscv_rvc_jump;
    } else if (MIFrm == TinyRISCVII::InstFormatCB) {
      FixupKind = TinyRISCV::fixup_riscv_rvc_branch;
    }
  }

  assert(FixupKind != TinyRISCV::fixup_riscv_invalid && "Unhandled expression!");

  Fixups.push_back(
      MCFixup::create(0, Expr, MCFixupKind(FixupKind), MI.getLoc()));
  ++MCNumFixups;

  return 0;
}

#include "TinyRISCVGenMCCodeEmitter.inc"
