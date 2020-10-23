//
// Created by pedro-teixeira on 29/09/2020.
//
#include "TinyRISCVTargetMachine.h"
#include "TargetInfo/TinyRISCVTargetInfo.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

#define DEBUG_TYPE "asm-printer"

namespace {
class TinyRISCVAsmPrinter : public AsmPrinter {
public:
  explicit TinyRISCVAsmPrinter(TargetMachine &TM,
                           std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer)) {}

  StringRef getPassName() const override { return "TinyRISCV Assembly Printer"; }

  void EmitInstruction(const MachineInstr *MI) override;

};

void TinyRISCVAsmPrinter::EmitInstruction(const MachineInstr *MI) {
  MCInst TmpInst;
  LowerTinyRISCVMachineInstrToMCInst(MI, TmpInst, *this);
  EmitToStreamer(*OutStreamer, TmpInst);
}

} // end namespace

// Force static initialization.
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeTinyRISCVAsmPrinter() {
  RegisterAsmPrinter<TinyRISCVAsmPrinter> X(getTheTinyRISCVTarget());
}
