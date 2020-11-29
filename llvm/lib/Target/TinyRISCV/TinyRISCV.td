//===----------------------------------------------------------------------===//
// Target-independent interfaces which we are implementing
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

//===----------------------------------------------------------------------===//
// TinyRISCV has no Subtarget features.
//===----------------------------------------------------------------------===//


//===----------------------------------------------------------------------===//
// Register File, Calling Conv, Instruction Descriptions
//===----------------------------------------------------------------------===//

include "TinyRISCVRegisterInfo.td"
include "TinyRISCVCallingConv.td"
include "TinyRISCVInstrInfo.td"

def TinyRISCVInstrInfo : InstrInfo;

//===----------------------------------------------------------------------===//
// TinyRISCV processors supported.
//===----------------------------------------------------------------------===//

class Proc<string Name, list<SubtargetFeature> Features>
 : Processor<Name, NoItineraries, Features>;

def : Proc<"generic", []>;

//===----------------------------------------------------------------------===//
// Declare the target which we are implementing
//===----------------------------------------------------------------------===//

def TinyRISCVAsmParser : AsmParser {
  let ShouldEmitMatchRegisterAltName = 1;
  let AllowDuplicateRegisterNames = 1;
}

def TinyRISCV : Target {
  // Pull in Instruction Info:
  let InstructionSet = TinyRISCVInstrInfo;
  let AssemblyParsers = [TinyRISCVAsmParser];
}
