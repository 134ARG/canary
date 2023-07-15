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

#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include "DyckAA/DyckAliasAnalysis.h"
#include "DyckAA/DyckGraph.h"
#include "DyckAA/DyckGraphNode.h"
#include "DyckAA/DyckVFG.h"
#include "Support/CFG.h"

DyckVFG::DyckVFG(DyckAliasAnalysis *DAA, Module *M) {
    // create a VFG for each function
    std::map<Function *, DyckVFG *> LocalVFGMap;
    for (auto &F: *M) {
        if (F.empty()) continue;
        LocalVFGMap[&F] = new DyckVFG(DAA, &F);
    }

    // todo connect local VFGs, delete local VFGs, maybe we can simplify the global VFG
}

DyckVFG::DyckVFG(DyckAliasAnalysis *DAA, Function *F) {
    // direct value flow through cast, gep-0-0, select, phi
    for (auto &I: instructions(*F)) {
        if (isa<CastInst>(I) || isa<PHINode>(I)) {
            auto *ToNode = getOrCreateVFGNode(&I);
            for (unsigned K = 0; K < I.getNumOperands(); ++K) {
                auto *From = I.getOperand(K);
                auto *FromNode = getOrCreateVFGNode(From);
                FromNode->addTarget(ToNode);
            }
        } else if (isa<SelectInst>(I)) {
            auto *ToNode = getOrCreateVFGNode(&I);
            for (unsigned K = 1; K < I.getNumOperands(); ++K) {
                auto *From = I.getOperand(K);
                auto *FromNode = getOrCreateVFGNode(From);
                FromNode->addTarget(ToNode);
            }
        } else if (auto *GEP = dyn_cast<GetElementPtrInst>(&I)) {
            bool VF = true;
            for (auto &Index: GEP->indices()) {
                if (auto *CI = dyn_cast<ConstantInt>(&Index)) {
                    if (CI->getSExtValue() != 0) {
                        VF = false;
                        break;
                    }
                }
            }
            if (VF) {
                auto *ToNode = getOrCreateVFGNode(&I);
                auto *FromNode = getOrCreateVFGNode(GEP->getPointerOperand());
                FromNode->addTarget(ToNode);
            }
        }
    }

    // indirect value flow through load/store
    auto *DG = DAA->getDyckGraph();
    std::map<DyckGraphNode *, std::vector<LoadInst *>> LoadMap; // ptr -> load
    std::map<DyckGraphNode *, std::vector<StoreInst *>> StoreMap; // ptr -> store
    for (auto &I: instructions(*F)) {
        if (auto *Load = dyn_cast<LoadInst>(&I)) {
            auto *Ptr = Load->getPointerOperand();
            auto *DV = DG->findDyckVertex(Ptr);
            if (DV) LoadMap[DV].push_back(Load);
        } else if (auto *Store = dyn_cast<StoreInst>(&I)) {
            auto *Ptr = Store->getPointerOperand();
            auto *DV = DG->findDyckVertex(Ptr);
            if (DV) StoreMap[DV].push_back(Store);
        }
    }
    // match load and store:
    // if alias(load's ptr, store's ptr) and store -> load in CFG, add store's value -> load's value in VFG
    CFG CtrlFlowG(F);
    for (auto &LoadIt: LoadMap) {
        auto *DyckNode = LoadIt.first;
        auto &Loads = LoadIt.second;
        auto StoreIt = StoreMap.find(DyckNode);
        if (StoreIt == StoreMap.end()) continue;
        auto &Stores = StoreIt->second;
        for (auto *Load: Loads) {
            auto *LdNode = getOrCreateVFGNode(Load);
            for (auto *Store: Stores)
                if (CtrlFlowG.reachable(Store, Load)) getOrCreateVFGNode(Store->getValueOperand())->addTarget(LdNode);
        }
    }
}

DyckVFG::~DyckVFG() {
    for (auto &It: ValueNodeMap) delete It.second;
}

DyckVFGNode *DyckVFG::getVFGNode(Value *V) {
    auto It = ValueNodeMap.find(V);
    if (It == ValueNodeMap.end()) return nullptr;
    return It->second;
}

DyckVFGNode *DyckVFG::getOrCreateVFGNode(Value *V) {
    auto It = ValueNodeMap.find(V);
    if (It == ValueNodeMap.end()) {
        auto *Ret = new DyckVFGNode(V);
        ValueNodeMap[V] = Ret;
        return Ret;
    }
    return It->second;
}