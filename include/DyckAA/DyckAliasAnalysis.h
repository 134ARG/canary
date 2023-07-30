/*
 *  Canary features a fast unification-based alias analysis for C programs
 *  Copyright (C) 2021 Qingkai Shi <qingkaishi@gmail.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef DYCKAA_DYCKALIASANALYSIS_H
#define DYCKAA_DYCKALIASANALYSIS_H

#include <llvm/Pass.h>
#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/IR/GetElementPtrTypeIterator.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/Debug.h>
#include <llvm/IR/InlineAsm.h>
#include <set>
#include "DyckAA/DyckCallGraph.h"
#include "DyckAA/DyckGraphEdgeLabel.h"
#include "DyckAA/DyckGraph.h"
#include "DyckAA/DyckVFG.h"

using namespace llvm;

class DyckAliasAnalysis : public ModulePass {
public:
    static char ID;

    DyckAliasAnalysis();

    ~DyckAliasAnalysis() override;

    bool runOnModule(Module &M) override;

    void getAnalysisUsage(AnalysisUsage &AU) const override;

    const std::set<Value *> *getAliasSet(Value *Ptr) const;

    bool mayAlias(Value *V1, Value *V2) const;

private:
    DyckGraph *CFLGraph;
    DyckCallGraph *DyckCG;
    DyckVFG *VFG;

private:
    /// Three kinds of information will be printed.
    /// 1. Alias Sets will be printed to the console
    /// 2. The relation of Alias Sets will be output into "alias_rel.dot"
    /// 3. The evaluation results will be output into "distribution.log"
    ///     The summary of the evaluation will be printed to the console
    void printAliasSetInformation(Module &M);

    /// escaped to the function Func
    void getEscapedPointersTo(std::set<DyckGraphNode *> *Ret, Function *Func);

    /// escaped from the pointer Pointer
    void getEscapedPointersFrom(std::set<DyckGraphNode *> *Ret, Value *From);

public:
    /// Get the vector of the may/must alias set that escape to 'func'
    void getEscapedPointersTo(std::vector<const std::set<Value *> *> *Ret, Function *Func);

    /// Get the vector of the may/must alias set that escape from 'from'
    void getEscapedPointersFrom(std::vector<const std::set<Value *> *> *Ret, Value *From);

    DyckCallGraph *getCallGraph() const;

    DyckGraph *getDyckGraph() const;

    /// Get the set of objects that a pointer may point to,
    /// e.g. for %a = load i32* %b, {%a} will be returned for the
    /// pointer %b
    void getPointstoObjects(std::set<Value *> &Objects, Value *Pointer) const;

    /// return a value flow graph created based on the alias analysis
    DyckVFG *getOrCreateValueFlowGraph(Module &M);
};

#endif // DYCKAA_DYCKALIASANALYSIS_H
