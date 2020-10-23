include "TinyRISCVInstrFormats.td"

//===----------------------------------------------------------------------===//
// Immediate definitions
//===----------------------------------------------------------------------===//
// TODO is this needed if we do not want a disassembler???

class ImmAsmOperand<string prefix, int width, string suffix> : AsmOperandClass {
  let Name = prefix # "Imm" # width # suffix;
  let RenderMethod = "addImmOperands";
  let DiagnosticType = !strconcat("Invalid", Name);
}

class SImmAsmOperand<int width, string suffix = "">
    : ImmAsmOperand<"S", width, suffix> {
}

def simm12 : Operand<i32>, ImmLeaf<i32, [{return isInt<12>(Imm);}]> {
  let ParserMatchClass = SImmAsmOperand<12>;
  let EncoderMethod = "getImmOpValue";
  let MCOperandPredicate = [{
    int64_t Imm;
    if (MCOp.evaluateAsConstantImm(Imm))
      return isInt<12>(Imm);
    return MCOp.isBareSymbolRef();
  }];
  let OperandType = "OPERAND_SIMM12";
  let OperandNamespace = "TinyRISCVOp";
}

// A 13-bit signed immediate where the least significant bit is zero.
def simm13_lsb0 : Operand<OtherVT> {
  let ParserMatchClass = SImmAsmOperand<13, "Lsb0">;
  let EncoderMethod = "getImmOpValueAsr1";
  let MCOperandPredicate = [{
    int64_t Imm;
    if (MCOp.evaluateAsConstantImm(Imm))
      return isShiftedInt<12, 1>(Imm);
    return MCOp.isBareSymbolRef();
  }];
  let OperandType = "OPERAND_SIMM13_LSB0";
  let OperandNamespace = "TinyRISCVOp";
}

def Simm21Lsb0JALAsmOperand : SImmAsmOperand<21, "Lsb0JAL"> {
  let ParserMethod = "parseJALOffset";
}

// A 21-bit signed immediate where the least significant bit is zero.
def simm21_lsb0_jal : Operand<OtherVT> {
  let ParserMatchClass = Simm21Lsb0JALAsmOperand;
  let EncoderMethod = "getImmOpValueAsr1";
  let MCOperandPredicate = [{
    int64_t Imm;
    if (MCOp.evaluateAsConstantImm(Imm))
      return isShiftedInt<20, 1>(Imm);
    return MCOp.isBareSymbolRef();
  }];
  let OperandType = "OPERAND_SIMM21_LSB0";
  let OperandNamespace = "TinyRISCVOp";
}

//===----------------------------------------------------------------------===//
// Instruction Definitions
//===----------------------------------------------------------------------===//

def ADD : TRVInstR<0b0000000, 0b000, OPC_OP, (outs GPR:$rd),
    (ins GPR:$rs1, GPR:$rs2),
    "add", "$rd, $rs1, $rs2">;

def MUL : TRVInstR<0b0000001, 0b000, OPC_OP, (outs GPR:$rd),
    (ins GPR:$rs1, GPR:$rs2),
    "mul", "$rd, $rs1, $rs2">;

def ADDI : TRVInstI<0b000, OPC_OP, (outs GPR:$rd),
    (ins GPR:$rs1, simm12:$imm12),
    "addi", "$rd, $rs1, $imm12">;

def LW : TRVInstI<0b010, OPC_LOAD, (outs GPR:$rd),
    (ins GPR:$rs1, simm12:$imm12),
    "lw", "$rd, ${imm12}(${rs1})">;

def SW : TRVInstS<0b010, OPC_STORE, (outs),
    (ins GPR:$rs2, GPR:$rs1, simm12:$imm12),
    "sw", "$rs2, ${imm12}(${rs1})">;

def BNE : TRVInstS<0b001, OPC_BRANCH, (outs),
    (ins GPR:$rs1, GPR:$rs2, simm13_lsb0:$imm12),
    "bne", "$rs1, $rs2, $imm12">;

let isCall = 1 in
def JAL : TRVInstJ<OPC_JAL, (outs GPR:$rd), (ins simm21_lsb0_jal:$imm20),
                  "jal", "$rd, $imm20">;

let isCall = 1 in
def JALR : TRVInstI<0b000, OPC_JALR, (outs GPR:$rd),
                   (ins GPR:$rs1, simm12:$imm12),
                   "jalr", "$rd, ${imm12}(${rs1})">;


//===----------------------------------------------------------------------===//
// Pattern Definitions
//===----------------------------------------------------------------------===

class PatGprGpr<SDPatternOperator OpNode, RVInst Inst>
    : Pat<(OpNode GPR:$rs1, GPR:$rs2), (Inst GPR:$rs1, GPR:$rs2)>;
class PatGprSimm12<SDPatternOperator OpNode, RVInstI Inst>
    : Pat<(OpNode GPR:$rs1, simm12:$imm12), (Inst GPR:$rs1, simm12:$imm12)>;

def : Pat<(simm12:$imm), (ADDI X0, simm12:$imm)>;

def : PatGprGpr<add, ADD>;
def : PatGprGpr<mul, MUL>;
def : PatGprSimm12<add, ADDI>;

