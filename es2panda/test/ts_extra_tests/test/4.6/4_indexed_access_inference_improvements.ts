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
 module: ESNext
 isCurrent: true
---*/


import { Assert } from '../../suite/assert.js'

interface HWI01 {
    number: number;
    string: string;
    boolean: boolean;
}

type HWI02<P extends keyof HWI01> = {
    [K in P]: {
        kind: K;
        v: HWI01[K];
        f: (p: HWI01[K]) => void;
    };
}[P];

function hwtest<K extends keyof HWI01>(record: HWI02<K>) {
    record.f(record.v);
}

hwtest({
    kind: "string",
    v: "hello!",
    f: (val) => {
        Assert.equal("HELLO!", val.toUpperCase());
    },
});