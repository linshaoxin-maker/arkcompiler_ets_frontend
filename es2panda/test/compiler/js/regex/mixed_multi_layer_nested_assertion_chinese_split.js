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
const regex = /^([\u4E00-\u9FA5])\B\W\b(?=\w\B(?=\d\b(?=\s\B(?=\W*\B(?=\D*\B(?=\S+\b(?![\p{N}])))))))/gmu;
const str = `
开开a1\r\n\tb
一_1   12
龥龥c3 龥龥龥\r\s
一_1   12
躯体a1\r\n\tb
一_1   12
结束
`;
const matches = str.split(regex);
print(JSON.stringify(matches));

