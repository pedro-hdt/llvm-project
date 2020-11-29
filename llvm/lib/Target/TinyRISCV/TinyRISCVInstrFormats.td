
class InstFormat<bits<5> val> {
  bits<5> Value = val;
}

def InstFormatPseudo : InstFormat<0>;
def InstFormatR      : InstFormat<1>;
def InstFormatI      : InstFormat<3>;
def InstFormatS      : InstFormat<4>;
def InstFormatU      : InstFormat<6>;
def InstFormatJ      : InstFormat<7>;

class TinyRISCVOpcode<bits<7> val> {
  bits<7> Value = val;
}

def OPC_LOAD      : TinyRISCVOpcode<0b0000011>;
def OPC_LOAD_FP   : TinyRISCVOpcode<0b0000111>;
def OPC_MISC_MEM  : TinyRISCVOpcode<0b0001111>;
def OPC_OP_IMM    : TinyRISCVOpcode<0b0010011>;
def OPC_AUIPC     : TinyRISCVOpcode<0b0010111>;
def OPC_OP_IMM_32 : TinyRISCVOpcode<0b0011011>;
def OPC_STORE     : TinyRISCVOpcode<0b0100011>;
def OPC_STORE_FP  : TinyRISCVOpcode<0b0100111>;
def OPC_AMO       : TinyRISCVOpcode<0b0101111>;
def OPC_OP        : TinyRISCVOpcode<0b0110011>;
def OPC_LUI       : TinyRISCVOpcode<0b0110111>;
def OPC_OP_32     : TinyRISCVOpcode<0b0111011>;
def OPC_MADD      : TinyRISCVOpcode<0b1000011>;
def OPC_MSUB      : TinyRISCVOpcode<0b1000111>;
def OPC_NMSUB     : TinyRISCVOpcode<0b1001011>;
def OPC_NMADD     : TinyRISCVOpcode<0b1001111>;
def OPC_OP_FP     : TinyRISCVOpcode<0b1010011>;
def OPC_BRANCH    : TinyRISCVOpcode<0b1100011>;
def OPC_JALR      : TinyRISCVOpcode<0b1100111>;
def OPC_JAL       : TinyRISCVOpcode<0b1101111>;
def OPC_SYSTEM    : TinyRISCVOpcode<0b1110011>;

class TRVInst<dag outs, dag ins, string opcodestr, string argstr,
              list<dag> pattern, InstFormat format>
    : Instruction {
    field bits<32> Inst;
    let Size = 4;

    // SoftFail is a field the disassembler can use to provide a way for
    // instructions to not match without killing the whole decode process. It is
    // mainly used for ARM, but Tablegen expects this field to exist or it fails
    // to build the decode table.
    field bits<32> SoftFail = 0;

    bits<7> Opcode = 0;

    let Inst{6-0} = Opcode;

    let Namespace = "TinyRISCV";

    dag OutOperandList = outs;
    dag InOperandList = ins;
    let AsmString = opcodestr # "\t" # argstr;
    let Pattern = pattern;

    let TSFlags{4-0} = format.Value;
}

class TRVInstR<bits<7> funct7, bits<3> funct3, TinyRISCVOpcode opcode, dag outs,
              dag ins, string opcodestr, string argstr>
    : TRVInst<outs, ins, opcodestr, argstr, [], InstFormatR> {
  bits<5> rs2;
  bits<5> rs1;
  bits<5> rd;

  let Inst{31-25} = funct7;
  let Inst{24-20} = rs2;
  let Inst{19-15} = rs1;
  let Inst{14-12} = funct3;
  let Inst{11-7} = rd;
  let Opcode = opcode.Value;
}

class TRVInstI<bits<3> funct3, TinyRISCVOpcode opcode, dag outs, dag ins,
              string opcodestr, string argstr>
    : TRVInst<outs, ins, opcodestr, argstr, [], InstFormatI> {
  bits<12> imm12;
  bits<5> rs1;
  bits<5> rd;

  let Inst{31-20} = imm12;
  let Inst{19-15} = rs1;
  let Inst{14-12} = funct3;
  let Inst{11-7} = rd;
  let Opcode = opcode.Value;
}

class TRVInstS<bits<3> funct3, TinyRISCVOpcode opcode, dag outs, dag ins,
              string opcodestr, string argstr>
    : TRVInst<outs, ins, opcodestr, argstr, [], InstFormatS> {
  bits<12> imm12;
  bits<5> rs2;
  bits<5> rs1;

  let Inst{31-25} = imm12{11-5};
  let Inst{24-20} = rs2;
  let Inst{19-15} = rs1;
  let Inst{14-12} = funct3;
  let Inst{11-7} = imm12{4-0};
  let Opcode = opcode.Value;
}

class TRVInstJ<TinyRISCVOpcode opcode, dag outs, dag ins, string opcodestr,
              string argstr>
    : TRVInst<outs, ins, opcodestr, argstr, [], InstFormatJ> {
  bits<20> imm20;
  bits<5> rd;

  let Inst{31} = imm20{19};
  let Inst{30-21} = imm20{9-0};
  let Inst{20} = imm20{10};
  let Inst{19-12} = imm20{18-11};
  let Inst{11-7} = rd;
  let Opcode = opcode.Value;
}

class TRVInstU<TinyRISCVOpcode opcode, dag outs, dag ins, string opcodestr,
              string argstr>
    : TRVInst<outs, ins, opcodestr, argstr, [], InstFormatU> {
  bits<20> imm20;
  bits<5> rd;

  let Inst{31-12} = imm20;
  let Inst{11-7} = rd;
  let Opcode = opcode.Value;
}

// pseudo instructions format
class TRVInstPseudo<dag outs, dag ins, list<dag> pattern, string opcodestr="", string argstr="">
    : TRVInst<outs, ins, opcodestr, argstr, pattern, InstFormatPseudo> {
  let isPseudo = 1;
  let isCodeGenOnly = 1;
}