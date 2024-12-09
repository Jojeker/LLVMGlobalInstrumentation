//=============================================================================
// Skeleton from: https://github.com/banach-space/llvm-tutor/
//
// FILE:
//    GlobalVarAccess.cpp
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

struct GlobalVarCounters : public PassInfoMixin<GlobalVarCounters> {
  // Main entry point, takes IR unit to run the pass on (&F) and the
  // corresponding pass manager (to be queried if need be)
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &) {
    errs() << "OK BOOTING UP\n";

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

    // // Write all the accesses to stdout (for now)
    // createDumpFunction(M, globalCounters);

    return PreservedAnalyses::all();
  }

  // void incrementCounter(
  //     GlobalVariable *globalVar,
  //     DenseMap<GlobalVariable *, std::tuple<GlobalVariable *, GlobalVariable *, GlobalVariable *>
  //     >
  //         &globalCounters,
  //     Instruction *insertPoint) {
  //   IRBuilder<> builder(insertPoint);
  //   GlobalVariable *counter;
  //
  //   errs() << "working with " << *globalVar;
  //   auto [rcnt, wcnt, ccnt] = globalCounters[globalVar];
  //
  //   // Load the current value of the counter and increment it
  //   if (dyn_cast<LoadInst>(insertPoint)) {
  //     errs() << "LD INSTRUCTION\n";
  //     counter = rcnt;
  //   } else if (dyn_cast<StoreInst>(insertPoint)) {
  //     errs() << "ST INSTRUCTION\n";
  //     counter = wcnt;
  //   } else if (dyn_cast<CallInst>(insertPoint)) {
  //     errs() << "CALL INSTRUCTION\n";
  //     counter = ccnt;
  //   }
  //
  //   if (!counter) errs() << "No type??? " << *insertPoint;
  //
  //   Value *intCounter =
  //       builder.CreateBitCast(counter, Type::getInt32Ty(builder.getContext())->getPointerTo());
  //   Value *counterVal = builder.CreateLoad(Type::getInt32Ty(builder.getContext()), intCounter);
  //   Value *incremented =
  //       builder.CreateAdd(counterVal, ConstantInt::get(Type::getInt32Ty(builder.getContext()),
  //       1));
  //   builder.CreateStore(incremented, counter);
  // }
  //
  // // Function to create the DUMP function in the module
  // void createDumpFunction(
  //     Module &M,
  //     DenseMap<GlobalVariable *, std::tuple<GlobalVariable *, GlobalVariable *, GlobalVariable *>
  //     >
  //         &globalCounters) {
  //   LLVMContext &Context = M.getContext();
  //   IRBuilder<> builder(Context);
  //
  //   // Define `void DUMP_GLOB()`
  //   Function *dumpFunc = M.getFunction("DUMP_GLOB");
  //
  //   // Get the defintion and replace it (no conflict resolution)
  //   if (!dumpFunc) {
  //     FunctionType *funcType = FunctionType::get(Type::getVoidTy(Context), false);
  //     dumpFunc = Function::Create(funcType, Function::ExternalLinkage, "DUMP_GLOB", M);
  //   }
  //
  //   // Create the entry block
  //   BasicBlock *entry = BasicBlock::Create(Context, "entry", dumpFunc);
  //   builder.SetInsertPoint(entry);
  //
  //   // Declare printf function
  //   FunctionCallee printfFunc = M.getOrInsertFunction(
  //       "printf", FunctionType::get(IntegerType::getInt32Ty(Context),
  //                                   {PointerType::get(Type::getInt8Ty(Context), 0)}, true));
  //
  //   // Format string for printing each variable and counter
  //   Value *format_str = builder.CreateGlobalStringPtr("%s=[%d:%d:%d]\n");
  //
  //   for (auto &entry : globalCounters) {
  //     GlobalVariable *globalVar = entry.first;
  //     auto [rd_counter, wr_counter, call_counter] = entry.second;
  //
  //     // Load the counter values and print them
  //     Value *rd_counterVal = builder.CreateLoad(rd_counter->getType(), rd_counter);
  //     Value *wr_counterVal = builder.CreateLoad(wr_counter->getType(), wr_counter);
  //     Value *call_counterVal = builder.CreateLoad(call_counter->getType(), call_counter);
  //
  //     builder.CreateCall(
  //         printfFunc,
  //         {format_str, builder.CreateGlobalStringPtr(demangle(globalVar->getName().str())),
  //          rd_counterVal, wr_counterVal, call_counterVal});
  //
  //     // Reset the counter (store ptr) for next time
  //     Value *zero = ConstantInt::get(Type::getInt32Ty(builder.getContext()), 0);
  //     builder.CreateStore(zero, rd_counter);
  //     builder.CreateStore(zero, wr_counter);
  //     builder.CreateStore(zero, call_counter);
  //   }
  //
  //   // Return from DUMP
  //   builder.CreateRetVoid();
  // }

  void insertCheckRangeGlobCall(IRBuilder<> &Builder, Value *ptr, Module *M) {
    // Create a call to checkRangeGlob(ptr)

    // Insert the call at the start of the current basic block
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
llvm::PassPluginLibraryInfo getGlobalVarAccessPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "GlobalVarCounters", LLVM_VERSION_STRING, [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &FPM, ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "global_var_counters") {
                    FPM.addPass(GlobalVarCounters());
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
  return getGlobalVarAccessPluginInfo();
}
