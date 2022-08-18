/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef MERGE_ABC_ANNOTATION_H
#define MERGE_ABC_ANNOTATION_H

#include "assembly-program.h"
#include "assemblyType.h"
#include "annotation.pb.h"
#include "arena_allocator.h"

namespace panda::proto {
class AnnotationData {
public:
    static void Serialize(const panda::pandasm::AnnotationData &anno, proto_panda::AnnotationData &protoAnno);
    static void Deserialize(const proto_panda::AnnotationData &protoAnno, panda::pandasm::AnnotationData &anno);
};

class AnnotationElement {
public:
    explicit AnnotationElement()
        : allocator_(std::make_unique<panda::ArenaAllocator>(panda::SpaceType::SPACE_TYPE_COMPILER, nullptr, true))
    {
    }
    static void Serialize(const panda::pandasm::AnnotationElement &element,
                                        proto_panda::AnnotationElement &protoElement);
    panda::pandasm::AnnotationElement &Deserialize(const proto_panda::AnnotationElement &protoElement);
private:
    std::unique_ptr<panda::ArenaAllocator> allocator_ {};
};

class ScalarValue {
public:
    explicit ScalarValue()
        : allocator_(std::make_unique<panda::ArenaAllocator>(panda::SpaceType::SPACE_TYPE_COMPILER, nullptr, true))
    {
    }
    static void Serialize(const panda::pandasm::ScalarValue &scalar, proto_panda::ScalarValue &protoScalar);
    panda::pandasm::ScalarValue Deserialize(const proto_panda::ScalarValue &protoScalar);
    static panda::pandasm::ScalarValue CreateScalarValue(const panda::pandasm::Value::Type &type,
        std::variant<uint64_t, float, double, std::string, panda::pandasm::Type, panda::pandasm::AnnotationData>
        &value);
private:
    std::unique_ptr<panda::ArenaAllocator> allocator_ {};
};

class ArrayValue {
public:
    explicit ArrayValue()
        : allocator_(std::make_unique<panda::ArenaAllocator>(panda::SpaceType::SPACE_TYPE_COMPILER, nullptr, true))
    {
    }
    static void Serialize(const panda::pandasm::ArrayValue &array, proto_panda::ArrayValue &protoArray);
    panda::pandasm::ArrayValue &Deserialize(const proto_panda::ArrayValue &protoArray);
private:
    std::unique_ptr<panda::ArenaAllocator> allocator_ {};
};
} // panda::proto
#endif
