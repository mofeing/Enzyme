//===- PrintAliasAnalysis.cpp - Pass to print alias analysis --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements a pass to print the results of running alias
// analysis.
//===----------------------------------------------------------------------===//

#include "Analysis/AliasAnalysis.h"
#include "Dialect/Ops.h"
#include "Passes/PassDetails.h"
#include "Passes/Passes.h"

#include "mlir/Analysis/DataFlow/ConstantPropagationAnalysis.h"
#include "mlir/Analysis/DataFlow/DeadCodeAnalysis.h"
#include "mlir/Interfaces/FunctionInterfaces.h"

using namespace mlir;

namespace {
using llvm::errs;

struct PrintAliasAnalysisPass
    : public enzyme::PrintAliasAnalysisPassBase<PrintAliasAnalysisPass> {

  void runOnOperation() override {
    DataFlowSolver solver;

    solver.load<enzyme::AliasAnalysis>(&getContext());
    solver.load<enzyme::PointsToPointerAnalysis>();
    solver.load<dataflow::DeadCodeAnalysis>();
    solver.load<dataflow::SparseConstantPropagation>();

    if (failed(solver.initializeAndRun(getOperation()))) {
      return signalPassFailure();
    }

    SmallVector<std::pair<Attribute, Value>> taggedPointers;
    getOperation()->walk([&](FunctionOpInterface funcOp) {
      for (auto arg : funcOp.getArguments()) {
        if (funcOp.getArgAttr(arg.getArgNumber(), "enzyme.tag")) {
          taggedPointers.push_back(
              {funcOp.getArgAttr(arg.getArgNumber(), "enzyme.tag"), arg});
        }
      }

      funcOp.walk([&](Operation *op) {
        if (op->hasAttr("tag")) {
          for (OpResult result : op->getResults()) {
            taggedPointers.push_back({op->getAttr("tag"), result});
          }
        }
      });
    });

    for (const auto &[tag, value] : taggedPointers) {
      const auto *state = solver.lookupState<enzyme::AliasClassLattice>(value);
      if (state) {
        errs() << "tag " << tag << " " << *state << "\n";
      }
    }

    if (taggedPointers.empty())
      return;

    // Compare all tagged pointers
    for (unsigned i = 0; i < taggedPointers.size() - 1; i++) {
      for (unsigned j = i + 1; j < taggedPointers.size(); j++) {
        const auto &[tagA, a] = taggedPointers[i];
        const auto &[tagB, b] = taggedPointers[j];

        const auto *lhs = solver.lookupState<enzyme::AliasClassLattice>(a);
        const auto *rhs = solver.lookupState<enzyme::AliasClassLattice>(b);
        if (!(lhs && rhs))
          continue;

        errs() << tagA << " and " << tagB << ": " << lhs->alias(*rhs) << "\n";
      }
    }

    getOperation()->walk([&solver](Operation *op) {
      if (auto funcOp = dyn_cast<FunctionOpInterface>(op)) {
        for (auto arg : funcOp.getArguments()) {
          auto *state = solver.lookupState<enzyme::AliasClassLattice>(arg);
          if (!state)
            continue;
          // TODO(zinenko): this has been overriding the argument...
          // Use an array attr instead (will break syntactic tests).
          state->getAliasClassesObject().foreachClass(
              [&](DistinctAttr aliasClass, enzyme::AliasClassSet::State state) {
                if (state == enzyme::AliasClassSet::State::Undefined)
                  funcOp.setArgAttr(
                      arg.getArgNumber(), "enzyme.ac",
                      StringAttr::get(arg.getContext(), "undefined"));
                else if (state == enzyme::AliasClassSet::State::Unknown)
                  funcOp.setArgAttr(
                      arg.getArgNumber(), "enzyme.ac",
                      StringAttr::get(arg.getContext(), "unknown"));
                else
                  funcOp.setArgAttr(arg.getArgNumber(), "enzyme.ac",
                                    aliasClass);
                return ChangeResult::NoChange;
              });
        }
      } else if (op->hasTrait<OpTrait::ReturnLike>() &&
                 isa<FunctionOpInterface>(op->getParentOp())) {
        errs() << "points-to-pointer sets for op @" << op->getLoc() << ":\n";
        if (auto *state = solver.lookupState<enzyme::PointsToSets>(op))
          errs() << *state << "\n";
        else
          errs() << "NOT computed\n";
      }
      if (op->hasAttr("tag")) {
        for (OpResult result : op->getResults()) {
          auto *state = solver.lookupState<enzyme::AliasClassLattice>(result);
          if (state) {
            if (state->isUnknown()) {
              op->setAttr("ac",
                          StringAttr::get(result.getContext(), "<unknown>"));
            } else {
              for (auto aliasClass : state->getAliasClasses()) {
                op->setAttr("ac", aliasClass);
              }
            }
          }
        }
      }
    });
  }
};
} // namespace

namespace mlir {
namespace enzyme {
std::unique_ptr<Pass> createPrintAliasAnalysisPass() {
  return std::make_unique<PrintAliasAnalysisPass>();
}
} // namespace enzyme
} // namespace mlir
