/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
const regex = /(?<=(?<![0-9]+)\p{N}{2}(?=[\cM\cI\cJ\cH\cI\cG]+)\D{2,4})\d{2}(?=\W*(?<=[\cM\cI\cJ\cH\cI\cG]?)\p{Nd}{2}(?!\d+))/gmu;
const str = `
s23\r\t\b45\x07\b12sd
a12\b\r6789
😁12\n\t\r16789
𝒜😁#79\t\r\b01\b89
`;
const matches = str.match(regex);
print(JSON.stringify(matches));

