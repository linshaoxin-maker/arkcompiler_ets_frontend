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
description: >
  Destructuring parameter declarations do not permit type annotations on the individual binding patterns, 
  as such annotations would conflict with the already established meaning of colons in object literals. 
  Type annotations must instead be written on the top-level parameter declaration.
 ---*/


interface DrawTextInfo {
    text?: string;
    location?: [number, number];
    bold?: boolean;
}

function drawText({ text, location, bold }: DrawTextInfo) {
    if (text) {
        Assert.equal(typeof text == "string", true);
    }
    if (location) {
        Assert.equal(Array.isArray(location), true);
    }
    if (bold) {
        Assert.equal(typeof bold === "boolean", true);
    }
}

drawText({ text: "text" });
drawText({ bold: false });
drawText({ location: [0, 1] });
