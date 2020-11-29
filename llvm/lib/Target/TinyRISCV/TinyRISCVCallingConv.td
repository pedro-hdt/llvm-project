//===----------------------------------------------------------------------===//
// TinyRISCV Return Value Calling Convention
//===----------------------------------------------------------------------===//
def RetCC_TinyRISCV : CallingConv<[
  // i32 are returned in registers X10 and X11
  CCIfType<[i32], CCAssignToReg<[X10, X11]>>,

  // Integer values get stored in stack slots that are 4 bytes in
  // size and 16-byte aligned.
  CCIfType<[i32], CCAssignToStack<4, 16>>
]>;

//===----------------------------------------------------------------------===//
// TinyRISCV Argument Calling Conventions
//===----------------------------------------------------------------------===//
def CC_TinyRISCV : CallingConv<[
  // Promote i8/i16 arguments to i32.
  CCIfType<[i8, i16], CCPromoteToType<i32>>,

  // The first 8 integer arguments are passed in integer registers.
  CCIfType<[i32], CCAssignToReg<[X10, X11, X12, X13, X14, X15, X16, X17]>>,

  // Integer values get stored in stack slots that are 4 bytes in
  // size and 16-byte aligned.
  CCIfType<[i32], CCAssignToStack<4, 16>>
]>;


// Calee Saved Registers in save order
def CSR_TinyRISCV
    : CalleeSavedRegs<(add X1, X3, X4, X8, X9, (sequence "X%u", 18, 27))>;