set(LLVM_TARGET_DEFINITIONS TinyRISCV.td)

tablegen(LLVM TinyRISCVGenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM TinyRISCVGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM TinyRISCVGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM TinyRISCVGenDisassemblerTables.inc -gen-disassembler)
tablegen(LLVM TinyRISCVGenGlobalISel.inc -gen-global-isel)
tablegen(LLVM TinyRISCVGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM TinyRISCVGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM TinyRISCVGenMCPseudoLowering.inc -gen-pseudo-lowering)
tablegen(LLVM TinyRISCVGenRegisterBank.inc -gen-register-bank)
tablegen(LLVM TinyRISCVGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM TinyRISCVGenSubtargetInfo.inc -gen-subtarget)

add_public_tablegen_target(TinyRISCVCommonTableGen)

add_llvm_target(TinyRISCVCodeGen
  TinyRISCVAsmPrinter.cpp
  TinyRISCVExpandPseudoInsts.cpp
  TinyRISCVFrameLowering.cpp
  TinyRISCVInstrInfo.cpp
  TinyRISCVISelDAGToDAG.cpp
  TinyRISCVISelLowering.cpp
  TinyRISCVMCInstLower.cpp
  TinyRISCVMergeBaseOffset.cpp
  TinyRISCVRegisterInfo.cpp
  TinyRISCVSubtarget.cpp
  TinyRISCVTargetMachine.cpp
  TinyRISCVTargetObjectFile.cpp
  TinyRISCVTargetTransformInfo.cpp
  )

add_subdirectory(AsmParser)
add_subdirectory(Disassembler)
add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
add_subdirectory(Utils)
