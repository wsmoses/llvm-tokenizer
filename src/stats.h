#pragma once

//===-- InstCount.cpp - Collects the count of all instructions ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This pass collects the count of all instructions and reports them
//
//===----------------------------------------------------------------------===//

#include <llvm/Analysis/Passes.h>
#include <llvm/ADT/Statistic.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstVisitor.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/Pass.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Analysis/CFG.h>
#include <llvm/IR/CFG.h>

#include <string>

using namespace llvm;

namespace {
  class InstCount : public InstVisitor<InstCount> {
    friend class InstVisitor<InstCount>;
public:
unsigned TotalInsts;
unsigned TotalBlocks;
unsigned BlockLow;
unsigned BlockMid;
unsigned BlockHigh;
unsigned TotalMemInst;
unsigned BeginPhi;
unsigned ArgsPhi;
unsigned BBNoPhi;
unsigned BB03Phi;
unsigned BBHiPhi;
unsigned BBNumArgsHi;
unsigned BBNumArgsLo;
unsigned testUnary;
unsigned binaryConstArg;
unsigned callLargeNumArgs;
unsigned returnInt;
unsigned oneSuccessor;
unsigned twoSuccessor;
unsigned moreSuccessors;
unsigned onePred;
unsigned twoPred;
unsigned morePreds;
unsigned onePredOneSuc;
unsigned onePredTwoSuc;
unsigned twoPredOneSuc;
unsigned twoEach;
unsigned moreEach;
unsigned NumEdges;
unsigned CriticalCount;
unsigned BranchCount;
unsigned numConstOnes;
unsigned numConstZeroes;
unsigned const32Bit;
unsigned const64Bit;

#define HANDLE_INST(N, OPCODE, CLASS) \
unsigned Num ## OPCODE ## Inst;
#include "llvm/IR/Instruction.def"
#undef HANDLE_INST

unsigned UncondBranches;

InstCount() :
TotalInsts(0),
TotalBlocks(0),
BlockLow(0),
BlockMid(0),
BlockHigh(0),
TotalMemInst(0),
BeginPhi(0),
ArgsPhi(0),
BBNoPhi(0),
BB03Phi(0),
BBHiPhi(0),
BBNumArgsHi(0),
BBNumArgsLo(0),
testUnary(0),
binaryConstArg(0),
callLargeNumArgs(0),
returnInt(0),
oneSuccessor(0),
twoSuccessor(0),
moreSuccessors(0),
onePred(0),
twoPred(0),
morePreds(0),
onePredOneSuc(0),
onePredTwoSuc(0),
twoPredOneSuc(0),
twoEach(0),
moreEach(0),
NumEdges(0),
CriticalCount(0),
BranchCount(0),
numConstOnes(0),
numConstZeroes(0),
const32Bit(0),
const64Bit(0),
#define HANDLE_INST(N, OPCODE, CLASS) \
Num ## OPCODE ## Inst(0),
#include "llvm/IR/Instruction.def"
#undef HANDLE_INST
UncondBranches(0)
{}
    void visitBasicBlock(BasicBlock &BB) { ++TotalBlocks;
    TerminatorInst* term = BB.getTerminator();
    unsigned numSuccessors = term->getNumSuccessors();
    for (int i = 0; i < numSuccessors; i++) {
        NumEdges++;
        if (isCriticalEdge(term, i)) {
            CriticalCount++;
        }
    }
    unsigned numPreds = 0;
    for (pred_iterator pi = pred_begin(&BB), E = pred_end(&BB); pi != E; ++pi) {
        numPreds++;
    }
    if (numSuccessors == 1) {
        oneSuccessor++;
    } else if (numSuccessors == 2) {
        twoSuccessor++;

    } else if (numSuccessors > 2) {
        moreSuccessors++;
    }
    if (numPreds == 1) {
        onePred++;
    } else if (numPreds == 2) {
        twoPred++;
    } else if (numPreds > 2) {
        morePreds++;
    }

    if (numPreds == 1 && numSuccessors == 1) {
        onePredOneSuc++;
    } else if (numPreds == 2 && numSuccessors == 1) {
        twoPredOneSuc++;
    } else if (numPreds == 1 && numSuccessors == 2) {
        onePredTwoSuc++;
    } else if (numPreds == 2 && numSuccessors == 2) {
        twoEach++;
    } else if (numPreds > 2 && numSuccessors > 2) {
        moreEach++;
    }

    unsigned tempCount = 0;
    bool isFirst = true;
    unsigned phiCount = 0;
    unsigned BBArgs = 0;
    for (Instruction &I : BB) {
        if (auto *bi = dyn_cast<BranchInst>(&I)) {
            BranchCount++;
            if (bi->isUnconditional()) {
                UncondBranches++;
            }
        }
        for (int i = 0 ; i < I.getNumOperands(); i++) {
            Value* v = I.getOperand(i);
            //Type* t = v->getType();
            if (auto *c = dyn_cast<Constant>(v)) {
                if (auto *ci = dyn_cast<ConstantInt>(c)) {
                    APInt val = ci->getValue();
                    unsigned bitWidth = val.getBitWidth();
                    if (bitWidth == 32) {
                        const32Bit++;
                    } else if (bitWidth == 64) {
                        const64Bit++;
                    }
                    if (val == 1) {
                        numConstOnes++;
                    } else if (val == 0) {
                        numConstZeroes++;
                    }
                }
            }
        }
        if (isa<CallInst>(I)) {
            if (cast<CallInst>(I).getNumArgOperands() > 4) {
                callLargeNumArgs++;
            }
            if (cast<CallInst>(I).getCalledFunction()->getReturnType()->isIntegerTy()) {
                returnInt++;
            }
        }
        if (isa<UnaryInstruction>(I)){
            testUnary++;
        }
        if (isa<BinaryOperator>(I)) {
            if (isa<Constant>(I.getOperand(0)) || isa<Constant>(I.getOperand(1))) {
            binaryConstArg++;

            }
        }
        if (isFirst && isa<PHINode>(I)) {
            BeginPhi++;
        }
        if (isa<PHINode>(I)) {
            phiCount++;
            unsigned inc = cast<PHINode>(I).getNumIncomingValues();
            ArgsPhi += inc;
            BBArgs += inc;
        }
        isFirst = false;
        tempCount++;
    }
    if (phiCount == 0) {
        BBNoPhi++;
    } else if (phiCount <= 3) {
        BB03Phi++;
    } else {
        BBHiPhi++;
    }
    if (BBArgs > 5) {
        BBNumArgsHi++;
    } else if (BBArgs>=1) {
        BBNumArgsLo++;
    }
    if (tempCount <15) {
        BlockLow++;
    } else if (tempCount <= 500) {
        BlockMid++;
    } else {
        BlockHigh++;
    }
    }

#define HANDLE_INST(N, OPCODE, CLASS) \
    void visit##OPCODE(CLASS &) { ++Num##OPCODE##Inst; ++TotalInsts; }

#include "llvm/IR/Instruction.def"

    void visitInstruction(Instruction &I) {
      errs() << "Instruction Count does not know about " << I;
      llvm_unreachable(nullptr);
    }
  };
}



std::map<std::string,int> getStats(Function &F) {
  InstCount visitor;
  visitor.visit(F);
  unsigned StartMemInsts =
    visitor.NumGetElementPtrInst + visitor.NumLoadInst + visitor.NumStoreInst + visitor.NumCallInst +
    visitor.NumInvokeInst + visitor.NumAllocaInst;
  unsigned EndMemInsts =
    visitor.NumGetElementPtrInst + visitor.NumLoadInst + visitor.NumStoreInst + visitor.NumCallInst +
    visitor.NumInvokeInst + visitor.NumAllocaInst;
  visitor.TotalMemInst += EndMemInsts-StartMemInsts;

  std::map<std::string,int> data;

data["TotalInsts"] = visitor.TotalInsts;
data["TotalBlocks"] = visitor.TotalBlocks;
data["BlockLow"] = visitor.BlockLow;
data["BlockMid"] = visitor.BlockMid;
data["BlockHigh"] = visitor.BlockHigh;
data["TotalMemInst"] = visitor.TotalMemInst;
data["BeginPhi"] = visitor.BeginPhi;
data["ArgsPhi"] = visitor.ArgsPhi;
data["BBNoPhi"] = visitor.BBNoPhi;
data["BB03Phi"] = visitor.BB03Phi;
data["BBHiPhi"] = visitor.BBHiPhi;
data["BBNumArgsHi"] = visitor.BBNumArgsHi;
data["BBNumArgsLo"] = visitor.BBNumArgsLo;
data["testUnary"] = visitor.testUnary;
data["binaryConstArg"] = visitor.binaryConstArg;
data["callLargeNumArgs"] = visitor.callLargeNumArgs;
data["returnInt"] = visitor.returnInt;
data["oneSuccessor"] = visitor.oneSuccessor;
data["twoSuccessor"] = visitor.twoSuccessor;
data["moreSuccessors"] = visitor.moreSuccessors;
data["onePred"] = visitor.onePred;
data["twoPred"] = visitor.twoPred;
data["morePreds"] = visitor.morePreds;
data["onePredOneSuc"] = visitor.onePredOneSuc;
data["onePredTwoSuc"] = visitor.onePredTwoSuc;
data["twoPredOneSuc"] = visitor.twoPredOneSuc;
data["twoEach"] = visitor.twoEach;
data["moreEach"] = visitor.moreEach;
data["NumEdges"] = visitor.NumEdges;
data["CriticalCount"] = visitor.CriticalCount;
data["BranchCount"] = visitor.BranchCount;
data["numConstOnes"] = visitor.numConstOnes;
data["numConstZeroes"] = visitor.numConstZeroes;
data["const32Bit"] = visitor.const32Bit;
data["const64Bit"] = visitor.const64Bit;
data["UncondBranche"] = visitor.UncondBranches;
#define HANDLE_INST(N, OPCODE, CLASS) \
data["Num" #OPCODE "Inst"] = visitor.Num ## OPCODE ## Inst;
#include "llvm/IR/Instruction.def"
#undef HANDLE_INST

  return data;
}
