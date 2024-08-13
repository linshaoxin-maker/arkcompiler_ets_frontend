"use strict";

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.mergeArrayMaps = mergeArrayMaps;
function _newArrowCheck(n, r) { if (n !== r) throw new TypeError("Cannot instantiate an arrow function"); }
/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

function mergeArrayMaps(lhs, rhs) {
  var _this = this;
  if (lhs.size === 0) {
    return rhs;
  }
  if (rhs.size === 0) {
    return lhs;
  }
  rhs.forEach(function (values, key) {
    _newArrowCheck(this, _this);
    if (values.length !== 0) {
      if (lhs.has(key)) {
        lhs.get(key).push(...values);
      } else {
        lhs.set(key, values);
      }
    }
  }.bind(this));
  return lhs;
}
//# sourceMappingURL=MergeArrayMaps.js.map