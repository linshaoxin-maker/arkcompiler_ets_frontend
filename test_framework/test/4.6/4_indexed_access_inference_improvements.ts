/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/**---
 description: TypeScript now can correctly infer to indexed access types which immediately index into a mapped object type.
---*/


interface TypeMap {
    number: number;
    string: string;
    boolean: boolean;
}

type UnionRecord<P extends keyof TypeMap> = {
    [K in P]: {
        kind: K;
        v: TypeMap[K];
        f: (p: TypeMap[K]) => void;
    };
}[P];

function processRecord<K extends keyof TypeMap>(record: UnionRecord<K>) {
    record.f(record.v);
}

// This call used to have issues - now works!
processRecord({
    kind: "string",
    v: "hello!",

    // 'val' used to implicitly have the type 'string | number | boolean',
    // but now is correctly inferred to just 'string'.
    f: (val) => {
        Assert.equal("HELLO!", val.toUpperCase());
    },
});