include "TinyRISCVInstrFormats.td"

//===----------------------------------------------------------------------===//
// Type requirements and profiles
//===----------------------------------------------------------------------===//

def SDT_TinyRISCVCallSeqStart : SDCallSeqStart<[ SDTCisVT<0, i32> ]>;

def SDT_TinyRISCVCallSeqEnd   : SDCallSeqEnd<[SDTCisVT<0, i32>,
                                              SDTCisVT<1, i32>]>;

def SDT_TinyRISCVCall : SDTypeProfile<0, -1, [SDTCisVT<0, i32>]>;

def SDT_TinyRISCVSelectCC : SDTypeProfile<1, 5, [SDTCisSameAs<1, 2>,
                                             SDTCisSameAs<0, 4>,
                                             SDTCisSameAs<4, 5>]>;

//===----------------------------------------------------------------------===//
// Generic nodes with target-specific formats
//===----------------------------------------------------------------------===//

def callseq_start : SDNode<"ISD::CALLSEQ_START", SDT_TinyRISCVCallSeqStart,
                           [SDNPHasChain, SDNPOutGlue]>;
def callseq_end   : SDNode<"ISD::CALLSEQ_END",   SDT_TinyRISCVCallSeqEnd,
                           [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

//===----------------------------------------------------------------------===//
// Custom SDNodes.
//===----------------------------------------------------------------------===//

def tinyriscv_call
    : SDNode<"TinyRISCVISD::CALL", SDT_TinyRISCVCall,
             [ SDNPHasChain, SDNPOptInGlue, SDNPOutGlue, SDNPVariadic ]>;

def tinyriscv_ret_flag  : SDNode<"TinyRISCVISD::RET_FLAG", SDTNone,
                             [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

def tinyriscv_selectcc  : SDNode<"TinyRISCVISD::SELECT_CC", SDT_TinyRISCVSelectCC,
                             [SDNPInGlue]>;

//===----------------------------------------------------------------------===//
// Operand definitions
//===----------------------------------------------------------------------===//

// A 12-bit signed immediate, the most common type in RISCV
def simm12 : Operand<i32>, ImmLeaf<i32, [{return isInt<12>(Imm);}]> {
  let MCOperandPredicate = [{
    int64_t Imm;
    if (MCOp.evaluateAsConstantImm(Imm))
      return isInt<12>(Imm);
    return MCOp.isBareSymbolRef();
  }];
}

// A 13-bit signed immediate where the least significant bit is zero.
def simm13_lsb0 : Operand<OtherVT> {
  let MCOperandPredicate = [{
    int64_t Imm;
    if (MCOp.evaluateAsConstantImm(Imm))
      return isShiftedInt<12, 1>(Imm);
    return MCOp.isBareSymbolRef();
  }];
}

// A 21-bit signed immediate where the least significant bit is zero.
def simm21_lsb0_jal : Operand<OtherVT> {
  let MCOperandPredicate = [{
    int64_t Imm;
    if (MCOp.evaluateAsConstantImm(Imm))
      return isShiftedInt<20, 1>(Imm);
    return MCOp.isBareSymbolRef();
  }];
}

// A 20-bit unsigned immediate
class UImm20Operand : Operand<i32> {
  let MCOperandPredicate = [{
    int64_t Imm;
    if (MCOp.evaluateAsConstantImm(Imm))
      return isUInt<20>(Imm);
    return MCOp.isBareSymbolRef();
  }];
}
// TODO these are the same for now but may need diff characteristics
def uimm20_lui : UImm20Operand;
def uimm20_auipc : UImm20Operand;

// A 5-bit unsigned immediate
def uimm5 : Operand<i32>, ImmLeaf<i32, [{return isUInt<5>(Imm);}]>;

// A call symbol.
def call_symbol : Operand<i32>;

// A bare symbol.
def bare_symbol : Operand<i32>;

// A bare symbol with the %tprel_add variant.
def tprel_add_symbol : Operand<i32>;

//===----------------------------------------------------------------------===//
// Instruction Definitions
//===----------------------------------------------------------------------===//

// Arithmetic:

def ADD : TRVInstR<0b0000000, 0b000, OPC_OP, (outs GPR:$rd),
    (ins GPR:$rs1, GPR:$rs2),
    "add", "$rd, $rs1, $rs2">;

def SUB : TRVInstR<0b0100000, 0b000, OPC_OP, (outs GPR:$rd),
    (ins GPR:$rs1, GPR:$rs2),
    "sub", "$rd, $rs1, $rs2">;

def MUL : TRVInstR<0b0000001, 0b000, OPC_OP, (outs GPR:$rd),
    (ins GPR:$rs1, GPR:$rs2),
    "mul", "$rd, $rs1, $rs2">;

def ADDI : TRVInstI<0b000, OPC_OP_IMM, (outs GPR:$rd),
    (ins GPR:$rs1, simm12:$imm12),
    "addi", "$rd, $rs1, $imm12">;

// Logic:

def AND : TRVInstR<0b0000000, 0b111, OPC_OP, (outs GPR:$rd),
    (ins GPR:$rs1, GPR:$rs2),
    "and", "$rd, $rs1, $rs2">;

def OR : TRVInstR<0b0000000, 0b110, OPC_OP, (outs GPR:$rd),
    (ins GPR:$rs1, GPR:$rs2),
    "or", "$rd, $rs1, $rs2">;

def XOR : TRVInstR<0b0000000, 0b100, OPC_OP, (outs GPR:$rd),
    (ins GPR:$rs1, GPR:$rs2),
    "xor", "$rd, $rs1, $rs2">;

def ANDI : TRVInstI<0b111, OPC_OP_IMM, (outs GPR:$rd),
    (ins GPR:$rs1, simm12:$imm12),
    "andi", "$rd, $rs1, $imm12">;

def ORI : TRVInstI<0b110, OPC_OP_IMM, (outs GPR:$rd),
    (ins GPR:$rs1, simm12:$imm12),
    "ori", "$rd, $rs1, $imm12">;

def XORI : TRVInstI<0b100, OPC_OP_IMM, (outs GPR:$rd),
    (ins GPR:$rs1, simm12:$imm12),
    "xori", "$rd, $rs1, $imm12">;

def SLT : TRVInstR<0b0000000, 0b010, OPC_OP, (outs GPR:$rd),
    (ins GPR:$rs1, GPR:$rs2),
    "slt", "$rd, $rs1, $rs2">;

def SLTU : TRVInstR<0b0000000, 0b011, OPC_OP, (outs GPR:$rd),
    (ins GPR:$rs1, GPR:$rs2),
    "sltu", "$rd, $rs1, $rs2">;

// Shifts:

def SRA : TRVInstR<0b0100000, 0b101, OPC_OP, (outs GPR:$rd),
    (ins GPR:$rs1, GPR:$rs2),
    "sra", "$rd, $rs1, $rs2">;

def SRL : TRVInstR<0b0000000, 0b101, OPC_OP, (outs GPR:$rd),
    (ins GPR:$rs1, GPR:$rs2),
    "srl", "$rd, $rs1, $rs2">;

def SLL : TRVInstR<0b0000000, 0b001, OPC_OP, (outs GPR:$rd),
    (ins GPR:$rs1, GPR:$rs2),
    "sll", "$rd, $rs1, $rs2">;

def SLTI : TRVInstI<0b010, OPC_OP_IMM, (outs GPR:$rd),
    (ins GPR:$rs1, simm12:$imm12),
    "slti", "$rd, $rs1, $imm12">;

def SLTIU : TRVInstI<0b011, OPC_OP_IMM, (outs GPR:$rd),
    (ins GPR:$rs1, simm12:$imm12),
    "sltiu", "$rd, $rs1, $imm12">;

def SRAI : TRVInstR<0b0100000, 0b101, OPC_OP_IMM, (outs GPR:$rd),
    (ins GPR:$rs1, uimm5:$imm5),
    "srai", "$rd, $rs1, $imm5">;

def SRLI : TRVInstR<0b0000000, 0b101, OPC_OP_IMM, (outs GPR:$rd),
    (ins GPR:$rs1, uimm5:$imm5),
    "srli", "$rd, $rs1, $imm5">;

def SLLI : TRVInstR<0b0000000, 0b001, OPC_OP_IMM, (outs GPR:$rd),
    (ins GPR:$rs1, uimm5:$imm5),
    "slli", "$rd, $rs1, $imm5">;

// Upper imm manip and PC:

def LUI : TRVInstU<OPC_LUI, (outs GPR:$rd),
    (ins uimm20_lui:$imm20),
    "lui", "$rd, $imm20">;

def AUIPC : TRVInstU<OPC_AUIPC, (outs GPR:$rd),
    (ins GPR:$rs1, uimm20_auipc:$imm20),
    "auipc", "$rd, $imm20">;

// Memory

def LW : TRVInstI<0b010, OPC_LOAD, (outs GPR:$rd),
    (ins GPR:$rs1, simm12:$imm12),
    "lw", "$rd, ${imm12}(${rs1})">;

def SW : TRVInstS<0b010, OPC_STORE, (outs),
    (ins GPR:$rs2, GPR:$rs1, simm12:$imm12),
    "sw", "$rs2, ${imm12}(${rs1})">;

// Branching (conditional)

def BNE : TRVInstS<0b001, OPC_BRANCH, (outs),
    (ins GPR:$rs1, GPR:$rs2, simm13_lsb0:$imm13),
    "bne", "$rs1, $rs2, $imm13">;

def BEQ : TRVInstS<0b000, OPC_BRANCH, (outs),
    (ins GPR:$rs1, GPR:$rs2, simm13_lsb0:$imm13),
    "beq", "$rs1, $rs2, $imm13">;

def BLT : TRVInstS<0b100, OPC_BRANCH, (outs),
    (ins GPR:$rs1, GPR:$rs2, simm13_lsb0:$imm13),
    "blt", "$rs1, $rs2, $imm13">;

def BGE : TRVInstS<0b101, OPC_BRANCH, (outs),
    (ins GPR:$rs1, GPR:$rs2, simm13_lsb0:$imm13),
    "bge", "$rs1, $rs2, $imm13">;

def BLTU : TRVInstS<0b110, OPC_BRANCH, (outs),
    (ins GPR:$rs1, GPR:$rs2, simm13_lsb0:$imm13),
    "bltu", "$rs1, $rs2, $imm13">;

def BGEU : TRVInstS<0b111, OPC_BRANCH, (outs),
    (ins GPR:$rs1, GPR:$rs2, simm13_lsb0:$imm13),
    "bgeu", "$rs1, $rs2, $imm13">;


// Jumps

let isCall = 1 in
def JAL : TRVInstJ<OPC_JAL, (outs GPR:$rd), (ins simm21_lsb0_jal:$imm20),
                  "jal", "$rd, $imm20">;

let isCall = 1 in
def JALR : TRVInstI<0b000, OPC_JALR, (outs GPR:$rd),
                   (ins GPR:$rs1, simm12:$imm12),
                   "jalr", "$rd, ${imm12}(${rs1})">;

//===----------------------------------------------------------------------===//
// Pattern Definitions for Instruction Selection
//===----------------------------------------------------------------------===

// for materializing immediate values
def : Pat<(simm12:$imm), (ADDI X0, simm12:$imm)>;

// reg reg patterns
class PatGprGpr<SDPatternOperator OpNode, TRVInst Inst>
    : Pat<(OpNode GPR:$rs1, GPR:$rs2), (Inst GPR:$rs1, GPR:$rs2)>;
def : PatGprGpr<add, ADD>;
def : PatGprGpr<mul, MUL>;
def : PatGprGpr<sub, SUB>;
def : PatGprGpr<and, AND>;
def : PatGprGpr<or, OR>;
def : PatGprGpr<xor, XOR>;

// This is a special case of the ADD instruction used to facilitate the use of a
// fourth operand to emit a relocation on a symbol relating to this instruction.
// The relocation does not affect any bits of the instruction itself but is used
// as a hint to the linker.
let hasSideEffects = 0, mayLoad = 0, mayStore = 0, isCodeGenOnly = 0 in
def PseudoAddTPRel : TRVInstPseudo<(outs GPR:$rd),
                            (ins GPR:$rs1, GPR:$rs2, tprel_add_symbol:$src), [],
                            "add", "$rd, $rs1, $rs2, $src">;

// reg imm patterns
class PatGprSimm12<SDPatternOperator OpNode, TRVInstI Inst>
    : Pat<(OpNode GPR:$rs1, simm12:$imm12), (Inst GPR:$rs1, simm12:$imm12)>;
def : PatGprSimm12<add, ADDI>;
def : PatGprSimm12<and, ANDI>;
def : PatGprSimm12<or, ORI>;
def : PatGprSimm12<xor, XORI>;
def : PatGprSimm12<and, ANDI>;

// Addressing modes.
// Necessary because a frameindex can't be matched directly in a pattern.
// def AddrFI : ComplexPattern<iPTR, 1, "SelectAddrFI", [frameindex], []>;
// TODO for FI addr support fix this

// mem patterns
multiclass LdPat<PatFrag LoadOp, TRVInst Inst> {
  def : Pat<(LoadOp GPR:$rs1), (Inst GPR:$rs1, 0)>;
//  def : Pat<(LoadOp AddrFI:$rs1), (Inst AddrFI:$rs1, 0)>;
  def : Pat<(LoadOp (add GPR:$rs1, simm12:$imm12)),
            (Inst GPR:$rs1, simm12:$imm12)>;
//  def : Pat<(LoadOp (add AddrFI:$rs1, simm12:$imm12)),
//            (Inst AddrFI:$rs1, simm12:$imm12)>; // TODO for FI addr support fix this
}
defm : LdPat<load, LW>;
multiclass StPat<PatFrag StoreOp, TRVInst Inst> {
  def : Pat<(StoreOp GPR:$rs2, GPR:$rs1), (Inst GPR:$rs2, GPR:$rs1, 0)>;
//  def : Pat<(StoreOp GPR:$rs2, AddrFI:$rs1), (Inst GPR:$rs2, AddrFI:$rs1, 0)>;
  def : Pat<(StoreOp GPR:$rs2, (add GPR:$rs1, simm12:$imm12)),
            (Inst GPR:$rs2, GPR:$rs1, simm12:$imm12)>;
//  def : Pat<(StoreOp GPR:$rs2, (add AddrFI:$rs1, simm12:$imm12)),
//            (Inst GPR:$rs2, AddrFI:$rs1, simm12:$imm12)>; // TODO for FI addr support fix this
}
defm : StPat<store, SW>;

let hasSideEffects = 0, mayLoad = 0, mayStore = 0, isCodeGenOnly = 0,
    isAsmParserOnly = 1 in
def PseudoLLA : TRVInstPseudo<(outs GPR:$dst), (ins bare_symbol:$src), [],
                       "lla", "$dst, $src">;

let hasSideEffects = 0, mayLoad = 1, mayStore = 0, isCodeGenOnly = 0,
    isAsmParserOnly = 1 in
def PseudoLA : TRVInstPseudo<(outs GPR:$dst), (ins bare_symbol:$src), [],
                      "la", "$dst, $src">;
let hasSideEffects = 0, mayLoad = 1, mayStore = 0, isCodeGenOnly = 0,
    isAsmParserOnly = 1 in
def PseudoLA_TLS_IE : TRVInstPseudo<(outs GPR:$dst), (ins bare_symbol:$src), [],
                             "la.tls.ie", "$dst, $src">;

let hasSideEffects = 0, mayLoad = 1, mayStore = 0, isCodeGenOnly = 0,
    isAsmParserOnly = 1 in
def PseudoLA_TLS_GD : TRVInstPseudo<(outs GPR:$dst), (ins bare_symbol:$src), [],
                             "la.tls.gd", "$dst, $src">;


// branch patterns
class BccPat<PatFrag CondOp, TRVInstS Inst>
    : Pat<(brcond (i32 (CondOp GPR:$rs1, GPR:$rs2)), bb:$imm12),
          (Inst GPR:$rs1, GPR:$rs2, simm13_lsb0:$imm12)>;
class BccSwapPat<PatFrag CondOp, TRVInstS InstBcc>
    : Pat<(brcond (i32 (CondOp GPR:$rs1, GPR:$rs2)), bb:$imm12),
          (InstBcc GPR:$rs2, GPR:$rs1, bb:$imm12)>;
def : BccPat<seteq, BEQ>;
def : BccPat<setne, BNE>;
def : BccPat<setlt, BLT>;
def : BccPat<setge, BGE>;
def : BccPat<setult, BLTU>;
def : BccPat<setuge, BGEU>;
def : BccSwapPat<setgt, BLT>;
def : BccSwapPat<setle, BGE>;
def : BccSwapPat<setugt, BLTU>;
def : BccSwapPat<setule, BGEU>;
def : Pat<(brcond GPR:$cond, bb:$imm12), (BNE GPR:$cond, X0, bb:$imm12)>;
let isBarrier = 1, isBranch = 1, isTerminator = 1 in
def PseudoBR : TRVInstPseudo<(outs), (ins simm21_lsb0_jal:$imm20), [(br bb:$imm20)], "j", "">,
               PseudoInstExpansion<(JAL X0, simm21_lsb0_jal:$imm20)>;
let isCall = 1, Defs=[X1] in
let isBarrier = 1, isBranch = 1, isIndirectBranch = 1, isTerminator = 1 in
def PseudoBRIND : TRVInstPseudo<(outs), (ins GPR:$rs1, simm12:$imm12), []>,
                  PseudoInstExpansion<(JALR X0, GPR:$rs1, simm12:$imm12)>;
def : Pat<(brind GPR:$rs1), (PseudoBRIND GPR:$rs1, 0)>;
def : Pat<(brind (add GPR:$rs1, simm12:$imm12)),
          (PseudoBRIND GPR:$rs1, simm12:$imm12)>;


// call patterns
let isCall = 1, Defs = [X1], isCodeGenOnly = 0 in
def PseudoCALL : TRVInstPseudo<(outs), (ins call_symbol:$func), []> {
  let AsmString = "call\t$func";
}
def : Pat<(tinyriscv_call tglobaladdr:$func), (PseudoCALL tglobaladdr:$func)>;
def : Pat<(tinyriscv_call texternalsym:$func), (PseudoCALL texternalsym:$func)>;
// PseudoCALLReg is a generic pseudo instruction for calls which will eventually
// expand to auipc and jalr while encoding, with any given register used as the
// destination.
// Define AsmString to print "call" when compile with -S flag.
// Define isCodeGenOnly = 0 to support parsing assembly "call" instruction.
let isCall = 1, isBarrier = 1, isCodeGenOnly = 0, hasSideEffects = 0,
    mayStore = 0, mayLoad = 0 in
def PseudoCALLReg : TRVInstPseudo<(outs GPR:$rd), (ins call_symbol:$func),
                                  [], "call", "$rd, $func"> {
}

// return pattern
let isTerminator = 1, isReturn = 1, isBarrier = 1, Uses = [X1] in {
def RET : TRVInstPseudo<(outs), (ins variable_ops), [(tinyriscv_ret_flag)], "ret", "">,
          PseudoInstExpansion<(JALR X0, X1, 0)>;
}
// ret = jalr x0, x1, 0
// x0 = x1 + 0 (result = pc+4 after exec but not needed)
// so this is like > /dev/null

//===----------------------------------------------------------------------===//
// Pseudo Instructions
//===----------------------------------------------------------------------===//

let Defs = [X2], Uses = [X2] in {
def ADJCALLSTACKDOWN : TRVInstPseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                                    [(callseq_start timm:$amt1, timm:$amt2)],
                                    "ADJCALLSTACKDOWN", "$amt1, $amt2">;
def ADJCALLSTACKUP   : TRVInstPseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                                    [(callseq_end timm:$amt1, timm:$amt2)],
                                    "ADJCALLSTACKUP", "$amt1, $amt2">;
}
