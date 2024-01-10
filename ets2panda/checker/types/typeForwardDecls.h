/**
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_TYPEFORWARDDECLS_H
#define PANDA_TYPEFORWARDDECLS_H

#include "macros.h"
#include "checker/types/typeMapping.h"

namespace panda::es2panda::checker {
class ETSDynamicType;
class ETSAsyncFuncReturnType;
class ETSDynamicFunctionType;
class ETSTypeParameter;

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DECLARE_TYPENAMES(typeFlag, typeName) \
    class typeName;                           \
    using C##typeName = const typeName;
TYPE_MAPPING(DECLARE_TYPENAMES)
#undef DECLARE_TYPENAMES

class ETSStringType;
class ETSBigIntType;
class InterfaceType;
class Type;

using CETSStringType = const ETSStringType;
using CETSBigIntType = const ETSBigIntType;
using CInterfaceType = const InterfaceType;
using CheckerType = const Type;

}  // namespace panda::es2panda::checker
#endif  // PANDA_TYPEFORWARDDECLS_H
