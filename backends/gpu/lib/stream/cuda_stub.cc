// Copyright 2020 The TensorFlow Runtime Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

//===- cuda_stub.cc ---------------------------------------------*- C++ -*-===//
//
// Implementation of the CUDA API forwarding calls to symbols dynamically loaded
// from the real library.
//
//===----------------------------------------------------------------------===//

#include "cuda.h"  // from @cuda_headers
#include "symbol_loader.h"

// Memoizes load of the .so for this CUDA library.
static void *LoadSymbol(const char *symbol_name) {
  static SymbolLoader loader("libcuda.so");
  return loader.GetAddressOfSymbol(symbol_name);
}

template <typename Func>
static Func *GetFunctionPointer(const char *symbol_name, Func *func = nullptr) {
  return reinterpret_cast<Func *>(LoadSymbol(symbol_name));
}

// Calls function 'symbol_name' in shared library with 'args'.
// TODO(csigg): Change to 'auto Func' when C++17 is allowed.
template <typename Func, Func *, typename... Args>
static CUresult DynamicCall(const char *symbol_name, Args &&...args) {
  static auto func_ptr = GetFunctionPointer<Func>(symbol_name);
  if (!func_ptr) return CUDA_ERROR_SHARED_OBJECT_SYMBOL_NOT_FOUND;
  return func_ptr(std::forward<Args>(args)...);
}

#define CUDAAPI
#define __CUDA_DEPRECATED [[deprecated]]
#ifdef _WIN32
#define CUDA_CB __stdcall
#else
#define CUDA_CB
#endif

extern "C" {
#include "cuda_stub.cc.inc"
}  // extern "C"
