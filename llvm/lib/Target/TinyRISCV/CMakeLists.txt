set(LLVM_TARGET_DEFINITIONS TinyRISCV.td)

tablegen(LLVM TinyRISCVGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM TinyRISCVGenCallingConv.inc -gen-callingconv)
tablegen(LLVM TinyRISCVGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM TinyRISCVGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM TinyRISCVGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM TinyRISCVGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM TinyRISCVGenSubtargetInfo.inc -gen-subtarget)

add_public_tablegen_target(TinyRISCVCommonTableGen)

add_llvm_target(TinyRISCVCodeGen
  TinyRISCVAsmPrinter.cpp
  TinyRISCVFrameLowering.cpp
  TinyRISCVInstrInfo.cpp
  TinyRISCVISelDAGToDAG.cpp
  TinyRISCVISelLowering.cpp
  TinyRISCVMCInstLower.cpp
  TinyRISCVRegisterInfo.cpp
  TinyRISCVSubtarget.cpp
  TinyRISCVTargetMachine.cpp
  TinyRISCVTargetObjectFile.cpp
)

add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
