"use strict";

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.Logger = void 0;
function _defineProperty(e, r, t) { return (r = _toPropertyKey(r)) in e ? Object.defineProperty(e, r, { value: t, enumerable: !0, configurable: !0, writable: !0 }) : e[r] = t, e; }
function _toPropertyKey(t) { var i = _toPrimitive(t, "string"); return "symbol" == typeof i ? i : i + ""; }
function _toPrimitive(t, r) { if ("object" != typeof t || !t) return t; var e = t[Symbol.toPrimitive]; if (void 0 !== e) { var i = e.call(t, r || "default"); if ("object" != typeof i) return i; throw new TypeError("@@toPrimitive must return a primitive value."); } return ("string" === r ? String : Number)(t); }
/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

class Logger {
  static init(instance) {
    this.instance_ = instance;
  }
  static trace(message) {
    this.getInstance().trace(message);
  }
  static debug(message) {
    this.getInstance().debug(message);
  }
  static info(message) {
    this.getInstance().info(message);
  }
  static warn(message) {
    this.getInstance().warn(message);
  }
  static error(message) {
    this.getInstance().error(message);
  }
  static getInstance() {
    if (!this.instance_) {
      throw new Error('Not initialized');
    }
    return this.instance_;
  }
}
exports.Logger = Logger;
_defineProperty(Logger, "instance_", void 0);
//# sourceMappingURL=Logger.js.map