/* Copyright 2020 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

// This file defines the operations used in the tf_framework dialect.

#include "tensorflow/compiler/mlir/tools/kernel_gen/ir/tf_framework_ops.h"

#include "mlir/IR/Builders.h"  // from @llvm-project
#include "mlir/IR/DialectImplementation.h"  // from @llvm-project

namespace mlir {
#include "tensorflow/compiler/mlir/tools/kernel_gen/ir/tf_framework_structs.cc.inc"

namespace kernel_gen {
namespace tf_framework {

TFFrameworkDialect::TFFrameworkDialect(MLIRContext *context)
    : Dialect(getDialectNamespace(), context) {
  addOperations<
#define GET_OP_LIST
#include "tensorflow/compiler/mlir/tools/kernel_gen/ir/tf_framework_ops.cc.inc"
      >();
  addTypes<OpKernelContextType>();
}

/// Parse a type registered to this dialect.
Type TFFrameworkDialect::parseType(DialectAsmParser &parser) const {
  StringRef keyword;
  if (parser.parseKeyword(&keyword)) return Type();

  if (keyword == "op_kernel_context") {
    return OpKernelContextType::get(getContext());
  }

  parser.emitError(parser.getNameLoc(), "unknown TF Framework type: ")
      << keyword;
  return Type();
}

/// Print a type registered to this dialect.
void TFFrameworkDialect::printType(Type type, DialectAsmPrinter &os) const {
  switch (type.getKind()) {
    case TFFrameworkTypes::OpKernelContextType:
      os << "op_kernel_context";
      return;
    default:
      llvm_unreachable("unexpected TF Framework type kind");
  }
}

//===----------------------------------------------------------------------===//
// AllocLikeOp
//===----------------------------------------------------------------------===//
template <typename AllocLikeOp>
static LogicalResult Verify(AllocLikeOp op) {
  static_assert(llvm::is_one_of<AllocLikeOp, AllocOutputOp, AllocTempOp>::value,
                "applies to only alloc_output or alloc_temp");
  // Check that the total number of operands matches the number of dynamic
  // dimensions specified in the memref type.
  unsigned result_dyn_dims = op.getType().getNumDynamicDims();
  unsigned dyn_sizes_count = op.dyn_sizes().size();
  if (dyn_sizes_count != result_dyn_dims)
    return op.emitOpError()
           << "`dyn_sizes` count " << dyn_sizes_count
           << " does not match dynamic dimensions count in the result type"
           << op.getType();
  return success();
}

#define GET_OP_CLASSES
#include "tensorflow/compiler/mlir/tools/kernel_gen/ir/tf_framework_ops.cc.inc"

}  // namespace tf_framework
}  // namespace kernel_gen
}  // namespace mlir
