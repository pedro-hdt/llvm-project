//
// Created by pedro-teixeira on 30/09/2020.
//

#ifndef LLVM_LIB_TARGET_TinyRISCV_MCTARGETDESC_TinyRISCVMCEXPR_H
#define LLVM_LIB_TARGET_TinyRISCV_MCTARGETDESC_TinyRISCVMCEXPR_H

#include "llvm/MC/MCExpr.h"

namespace llvm {

class StringRef;
class MCOperand;
class TinyRISCVMCExpr : public MCTargetExpr {
public:
  enum VariantKind {
    VK_TinyRISCV_None,
    VK_TinyRISCV_LO,
    VK_TinyRISCV_HI,
    VK_TinyRISCV_Invalid
  };

private:
  const MCExpr *Expr;
  const VariantKind Kind;

  explicit TinyRISCVMCExpr(const MCExpr *Expr, VariantKind Kind)
      : Expr(Expr), Kind(Kind) {}

public:
  static const TinyRISCVMCExpr *create(const MCExpr *Expr, VariantKind Kind,
                                   MCContext &Ctx);

  VariantKind getKind() const { return Kind; }

  const MCExpr *getSubExpr() const { return Expr; }

  void printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const override;
  bool evaluateAsRelocatableImpl(MCValue &Res, const MCAsmLayout *Layout,
                                 const MCFixup *Fixup) const override;
  void visitUsedExpr(MCStreamer &Streamer) const override;
  MCFragment *findAssociatedFragment() const override {
    return getSubExpr()->findAssociatedFragment();
  }

  void fixELFSymbolsInTLSFixups(MCAssembler &Asm) const override;

  static bool classof(const MCExpr *E) {
    return E->getKind() == MCExpr::Target;
  }

  static bool classof(const TinyRISCVMCExpr *) { return true; }

  static VariantKind getVariantKindForName(StringRef name);
  static StringRef getVariantKindName(VariantKind Kind);
};

}

#endif // LLVM_LIB_TARGET_TinyRISCV_MCTARGETDESC_TinyRISCVMCEXPR_H
