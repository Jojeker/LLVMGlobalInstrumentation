//=============================================================================
// Skeleton from: https://github.com/banach-space/llvm-tutor/
//
// FILE:
//    GlobalAccessInstrumentation.cpp
//=============================================================================

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/ErrorHandling.h>

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

using namespace llvm;

namespace {

struct GlobalAccessInstrumentation : public PassInfoMixin<GlobalAccessInstrumentation> {
  // Main entry point, takes IR unit to run the pass on (&F) and the
  // corresponding pass manager (to be queried if need be)
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &) {
    outs() << "Running GlobalInstrumentation Pass...\n";

    // context and IR builder
    LLVMContext &Context = M.getContext();

    FunctionCallee checkRangeGlobFunc = M.getFunction("checkRangeGlob");
    if (!checkRangeGlobFunc) {
      report_fatal_error("Could not find 'checkRangeGlobFunc' defintion");
    }

    // Instrument every function that reads/writes a global
    for (Function &F : M) {
      // Skip declarations - only definitions and not checkRangeGlob
      if (F.isDeclaration()) continue;

      // Do not instrument checkRangeGlob
      if (F.getName() == "checkRangeGlob") continue;

      for (BasicBlock &BB : F) {
        for (Instruction &I : BB) {
          // Check the access type and increment accordingly
          Value *trueValue = ConstantInt::get(Type::getInt1Ty(M.getContext()), 1);
          Value *falseValue = ConstantInt::get(Type::getInt1Ty(M.getContext()), 0);

          // Get a debug location, otherwise LLVM complains
          DILocation *debugLocation = I.getDebugLoc();
          if (!debugLocation) {
            debugLocation = DILocation::get(M.getContext(), 0, 0, F.getSubprogram());
          }

          IRBuilder<> Builder(&I);

          if (LoadInst *loadInst = dyn_cast<LoadInst>(&I)) {
            CallInst *callInst =
                Builder.CreateCall(checkRangeGlobFunc, {loadInst->getPointerOperand(), trueValue});
            callInst->setDebugLoc(debugLocation);
          } else if (StoreInst *storeInst = dyn_cast<StoreInst>(&I)) {
            CallInst *callInst = Builder.CreateCall(checkRangeGlobFunc,
                                                    {storeInst->getPointerOperand(), falseValue});
            callInst->setDebugLoc(debugLocation);
          }
        }
      }
    }

    return PreservedAnalyses::all();
  }

  // Without isRequired returning true, this pass will be skipped for functions
  // decorated with the optnone LLVM attribute. Note that clang -O0 decorates
  // all functions with optnone.
  static bool isRequired() { return true; }
};
}  // namespace

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
llvm::PassPluginLibraryInfo getGlobalAccessInstrumentationPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "GlobalAccessInstrumentation", LLVM_VERSION_STRING, [](PassBuilder &PB) {
            PB.registerPipelineStartEPCallback(
                [](ModulePassManager &MPM, OptimizationLevel Level) {
                    MPM.addPass(GlobalAccessInstrumentation());
                }
            );
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM, ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "GlobalAccessInstrumentation") {
                    MPM.addPass(GlobalAccessInstrumentation());
                    return true;
                  }
                  return false;
                });
          }};
}

// This is the core interface for pass plugins. It guarantees that 'opt' will
// be able to recognize HelloWorld when added to the pass pipeline on the
// command line, i.e. via '-passes=hello-world'
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return getGlobalAccessInstrumentationPluginInfo();
}
