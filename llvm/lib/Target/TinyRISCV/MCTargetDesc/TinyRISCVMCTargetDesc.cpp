//
// Created by pedro-teixeira on 23/09/2020.
//

#include "TinyRISCVMCTargetDesc.h"
#include "MCTargetDesc/TinyRISCVInstPrinter.h"
#include "TinyRISCVMCAsmInfo.h"
#include "TargetInfo/TinyRISCVTargetInfo.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/TargetRegistry.h"

#define GET_INSTRINFO_MC_DESC
#include "TinyRISCVGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "TinyRISCVGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "TinyRISCVGenRegisterInfo.inc"

using namespace llvm;

static MCInstrInfo *createTinyRISCVMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitTinyRISCVMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createTinyRISCVMCRegisterInfo(StringRef TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitTinyRISCVMCRegisterInfo(X, TinyRISCV::LR);
  return X;
}

static MCAsmInfo *createTinyRISCVMCAsmInfo(const MCRegisterInfo &MRI, StringRef TT) {
  MCAsmInfo *MAI = new TinyRISCVMCAsmInfo(TT);
  return MAI;
}

static MCSubtargetInfo *createTinyRISCVMCSubtargetInfo(const Triple TT,
                                                 StringRef CPU, StringRef FS) {
  return createTinyRISCVMCSubtargetInfoImpl(TT, CPU, FS);
}

static MCInstPrinter *
createTinyRISCVMCInstPrinter(const Target &T, unsigned SyntaxVariant,
                       const MCAsmInfo &MAI, const MCInstrInfo &MII,
                       const MCRegisterInfo &MRI, const MCSubtargetInfo &STI) {
  return new TinyRISCVInstPrinter(MAI, MII, MRI);
}

static MCStreamer *
createMCAsmStreamer(MCContext &Ctx, formatted_raw_ostream *OS,
                    bool isVerboseAsm, bool useDwarfDirectory,
                    MCInstPrinter *InstPrint, MCCodeEmitter *CE,
                    MCAsmBackend *TAB, bool ShowInst) {
  return createAsmStreamer(Ctx, OS, isVerboseAsm, useDwarfDirectory,
                             InstPrint, CE, TAB, ShowInst);
}

static MCStreamer *createMCStreamer(const Target &T, StringRef TT,
                                    MCContext &Ctx, MCAsmBackend &MAB,
                                    raw_ostream &OS, MCCodeEmitter *Emitter,
                                    const MCSubtargetInfo &STI, bool RelaxAll,
                                    bool NoExecStack) {
  return createELFStreamer(Ctx, MAB, OS, Emitter, false);
}

// Force static initialization.
extern "C" void LLVMInitializeTinyRISCVTargetMC() {
  Target *T = &getTheTinyRISCVTarget();
  TargetRegistry::RegisterMCAsmInfo(*T, createTinyRISCVMCAsmInfo);
  TargetRegistry::RegisterMCInstrInfo(*T, createTinyRISCVMCInstrInfo);
  TargetRegistry::RegisterMCRegInfo(*T, createTinyRISCVMCRegisterInfo);
  TargetRegistry::RegisterMCAsmBackend(*T, createTinyRISCVAsmBackend);
  TargetRegistry::RegisterMCCodeEmitter(*T, createTinyRISCVMCCodeEmitter);
  TargetRegistry::RegisterMCInstPrinter(*T, createTinyRISCVMCInstPrinter);
  TargetRegistry::RegisterMCSubtargetInfo(*T, createTinyRISCVMCSubtargetInfo);
  // Do not create object file streamer

  // Register the asm target streamer.
  TargetRegistry::RegisterAsmTargetStreamer(*T, createMCAsmStreamer);
}