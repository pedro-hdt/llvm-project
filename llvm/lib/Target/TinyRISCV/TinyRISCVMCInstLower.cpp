//===-- TinyRISCVMCInstLower.cpp - Convert TinyRISCV MachineInstr to an MCInst ------=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains code to lower TinyRISCV MachineInstrs to their corresponding
// MCInst records.
//
//===----------------------------------------------------------------------===//

#include "TinyRISCV.h"
#include "MCTargetDesc/TinyRISCVMCExpr.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

static MCOperand lowerSymbolOperand(const MachineOperand &MO, MCSymbol *Sym,
                                    const AsmPrinter &AP) {
  MCContext &Ctx = AP.OutContext;
  TinyRISCVMCExpr::VariantKind Kind;

  switch (MO.getTargetFlags()) {
  default:
    llvm_unreachable("Unknown target flag on GV operand");
  case TinyRISCVII::MO_None:
    Kind = TinyRISCVMCExpr::VK_TinyRISCV_None;
    break;
  case TinyRISCVII::MO_CALL:
    Kind = TinyRISCVMCExpr::VK_TinyRISCV_CALL;
    break;
  case TinyRISCVII::MO_PLT:
    Kind = TinyRISCVMCExpr::VK_TinyRISCV_CALL_PLT;
    break;
  case TinyRISCVII::MO_LO:
    Kind = TinyRISCVMCExpr::VK_TinyRISCV_LO;
    break;
  case TinyRISCVII::MO_HI:
    Kind = TinyRISCVMCExpr::VK_TinyRISCV_HI;
    break;
  case TinyRISCVII::MO_PCREL_LO:
    Kind = TinyRISCVMCExpr::VK_TinyRISCV_PCREL_LO;
    break;
  case TinyRISCVII::MO_PCREL_HI:
    Kind = TinyRISCVMCExpr::VK_TinyRISCV_PCREL_HI;
    break;
  case TinyRISCVII::MO_GOT_HI:
    Kind = TinyRISCVMCExpr::VK_TinyRISCV_GOT_HI;
    break;
  case TinyRISCVII::MO_TPREL_LO:
    Kind = TinyRISCVMCExpr::VK_TinyRISCV_TPREL_LO;
    break;
  case TinyRISCVII::MO_TPREL_HI:
    Kind = TinyRISCVMCExpr::VK_TinyRISCV_TPREL_HI;
    break;
  case TinyRISCVII::MO_TPREL_ADD:
    Kind = TinyRISCVMCExpr::VK_TinyRISCV_TPREL_ADD;
    break;
  case TinyRISCVII::MO_TLS_GOT_HI:
    Kind = TinyRISCVMCExpr::VK_TinyRISCV_TLS_GOT_HI;
    break;
  case TinyRISCVII::MO_TLS_GD_HI:
    Kind = TinyRISCVMCExpr::VK_TinyRISCV_TLS_GD_HI;
    break;
  }

  const MCExpr *ME =
      MCSymbolRefExpr::create(Sym, MCSymbolRefExpr::VK_None, Ctx);

  if (!MO.isJTI() && !MO.isMBB() && MO.getOffset())
    ME = MCBinaryExpr::createAdd(
        ME, MCConstantExpr::create(MO.getOffset(), Ctx), Ctx);

  if (Kind != TinyRISCVMCExpr::VK_TinyRISCV_None)
    ME = TinyRISCVMCExpr::create(ME, Kind, Ctx);
  return MCOperand::createExpr(ME);
}

bool llvm::LowerTinyRISCVMachineOperandToMCOperand(const MachineOperand &MO,
                                               MCOperand &MCOp,
                                               const AsmPrinter &AP) {
  switch (MO.getType()) {
  default:
    report_fatal_error("LowerTinyRISCVMachineInstrToMCInst: unknown operand type");
  case MachineOperand::MO_Register:
    // Ignore all implicit register operands.
    if (MO.isImplicit())
      return false;
    MCOp = MCOperand::createReg(MO.getReg());
    break;
  case MachineOperand::MO_RegisterMask:
    // Regmasks are like implicit defs.
    return false;
  case MachineOperand::MO_Immediate:
    MCOp = MCOperand::createImm(MO.getImm());
    break;
  case MachineOperand::MO_MachineBasicBlock:
    MCOp = lowerSymbolOperand(MO, MO.getMBB()->getSymbol(), AP);
    break;
  case MachineOperand::MO_GlobalAddress:
    MCOp = lowerSymbolOperand(MO, AP.getSymbol(MO.getGlobal()), AP);
    break;
  case MachineOperand::MO_BlockAddress:
    MCOp = lowerSymbolOperand(
        MO, AP.GetBlockAddressSymbol(MO.getBlockAddress()), AP);
    break;
  case MachineOperand::MO_ExternalSymbol:
    MCOp = lowerSymbolOperand(
        MO, AP.GetExternalSymbolSymbol(MO.getSymbolName()), AP);
    break;
  case MachineOperand::MO_ConstantPoolIndex:
    MCOp = lowerSymbolOperand(MO, AP.GetCPISymbol(MO.getIndex()), AP);
    break;
  }
  return true;
}

void llvm::LowerTinyRISCVMachineInstrToMCInst(const MachineInstr *MI, MCInst &OutMI,
                                          const AsmPrinter &AP) {
  OutMI.setOpcode(MI->getOpcode());

  for (const MachineOperand &MO : MI->operands()) {
    MCOperand MCOp;
    if (LowerTinyRISCVMachineOperandToMCOperand(MO, MCOp, AP))
      OutMI.addOperand(MCOp);
  }
}
