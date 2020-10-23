def RetCC_TinyRISCV : CallingConv<[
    CCIfType<[i32], CCAssignToReg<[R0]>>,
    CCIfType<[i32], CCAssignToStack<4, 4>>
]>;

def CC_TinyRISCV : CallingConv<[
    CCIfType<[i8, i16], CCPromoteToType<i32>>,
    CCIfType<[i32], CCAssignToReg<[R0, R1]>>,
    CCIfType<[i32], CCAssignToStack<4, 4>>
]>;

def CSR
    : CalleeSavedRegs<(
        add X1, X3, X4, X8, X9,
        (sequence "X%u", 18, 27))>;