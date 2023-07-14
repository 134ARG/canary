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

#ifndef NULLPOINTER_NULLCHECKANALYSIS_H
#define NULLPOINTER_NULLCHECKANALYSIS_H

#include <llvm/IR/Function.h>
#include <llvm/Pass.h>
#include <list>
#include <set>
#include <unordered_map>

using namespace llvm;

class LocalNullCheckAnalysis;

class NullCheckAnalysis {
private:
    Pass *Driver;

    std::unordered_map<Function *, LocalNullCheckAnalysis *> AnalysisMap;

public:
    explicit NullCheckAnalysis(Pass *P) : Driver(P) {}

    ~NullCheckAnalysis();

    void run(Module &M);

    /// \p Ptr must be an operand of \p Inst
    /// return true if \p Ptr at \p Inst may be a null pointer
    bool mayNull(Value *Ptr, Instruction *Inst);
};

#endif // NULLPOINTER_NULLCHECKANALYSIS_H
