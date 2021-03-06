// Copyright 2019 The Clspv Authors. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "SPIRVOp.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"

#include "Builtins.h"
#include "Constants.h"

namespace clspv {

using namespace llvm;

Instruction *InsertSPIRVOp(Instruction *Insert, spv::Op Opcode,
                           ArrayRef<Attribute::AttrKind> Attributes,
                           Type *RetType, ArrayRef<Value *> Args) {

  // Prepare mangled name
  std::string MangledName = clspv::SPIRVOpIntrinsicFunction();
  MangledName += ".";
  MangledName += std::to_string(Opcode);
  MangledName += ".";
  for (auto Arg : Args) {
    MangledName += Builtins::GetMangledTypeName(Arg->getType());
  }

  // Create a function in the module
  auto M = Insert->getModule();
  auto Int32Ty = Type::getInt32Ty(M->getContext());
  SmallVector<Type *, 8> ArgTypes = {Int32Ty};
  for (auto Arg : Args) {
    ArgTypes.push_back(Arg->getType());
  }
  auto NewFType = FunctionType::get(RetType, ArgTypes, false);
  auto NewFTyC = M->getOrInsertFunction(MangledName, NewFType);
  auto NewF = cast<Function>(NewFTyC.getCallee());
  for (auto A : Attributes) {
    NewF->addFnAttr(A);
  }

  // Now call it with the values we were passed
  SmallVector<Value *, 8> ArgValues = {ConstantInt::get(Int32Ty, Opcode)};
  for (auto Arg : Args) {
    ArgValues.push_back(Arg);
  }

  return CallInst::Create(NewF, ArgValues, "", Insert);
}

}; // namespace clspv
