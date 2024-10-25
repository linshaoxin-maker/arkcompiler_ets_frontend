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
const regex = new RegExp(
    '(?<=(?<=(?<=(?<=(?<=(?<=(?<!\\p{Nd})\\b\\p{N})(?=\\W)' +
    '[\\cA\\cB\\cC\\cD\\cE\\cF\\cG\\cH\\cI\\cJ\\cK\\cL\\cM\\cN\\cO\\cP\\cQ\\cR\\cS\\cT\\cU\\cV\\cW\\cX\\cY\\cZ])' +
    '(?=\\S)\\B[\\u{1F300}-\\u{1F9FF}])(?=[\\r\\n\\t\\v\\f\\b])\\B\\s)(?=\\p{Lu})\\w)(?=[0-9]))\\d()',
    'ugm'
);
const str = `
①\x01😑\rP1
ⅱ1\x02😑\tQ2
3\x03😎\vq3
4\x04😏\bP4
6\r😏
P7
ⅱ\x05😎\rP5
`;
const matches = str.match(regex);
print(JSON.stringify(matches));

