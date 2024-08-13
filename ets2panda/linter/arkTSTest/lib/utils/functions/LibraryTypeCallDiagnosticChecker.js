"use strict";

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.TYPE_UNKNOWN_IS_NOT_ASSIGNABLE_TO_TYPE_1_RE = exports.TYPE_UNDEFINED_IS_NOT_ASSIGNABLE_TO_TYPE_1_RE = exports.TYPE_NULL_IS_NOT_ASSIGNABLE_TO_TYPE_1_RE = exports.TYPE_0_IS_NOT_ASSIGNABLE_TO_TYPE_1_ERROR_CODE = exports.OBJECT_IS_POSSIBLY_UNDEFINED_ERROR_CODE = exports.NO_OVERLOAD_MATCHES_THIS_CALL_ERROR_CODE = exports.LibraryTypeCallDiagnosticChecker = exports.ARGUMENT_OF_TYPE_UNDEFINED_IS_NOT_ASSIGNABLE_TO_PARAMETER_OF_TYPE_1_RE = exports.ARGUMENT_OF_TYPE_NULL_IS_NOT_ASSIGNABLE_TO_PARAMETER_OF_TYPE_1_RE = exports.ARGUMENT_OF_TYPE_0_IS_NOT_ASSIGNABLE_TO_PARAMETER_OF_TYPE_1_ERROR_CODE = void 0;
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

/*
 * Current approach relates on error code and error message matching and it is quite fragile,
 * so this place should be checked thoroughly in the case of typescript upgrade
 */
const TYPE_0_IS_NOT_ASSIGNABLE_TO_TYPE_1_ERROR_CODE = exports.TYPE_0_IS_NOT_ASSIGNABLE_TO_TYPE_1_ERROR_CODE = 2322;
const TYPE_UNKNOWN_IS_NOT_ASSIGNABLE_TO_TYPE_1_RE = exports.TYPE_UNKNOWN_IS_NOT_ASSIGNABLE_TO_TYPE_1_RE = /^Type '(.*)\bunknown\b(.*)' is not assignable to type '.*'\.$/;
const TYPE_NULL_IS_NOT_ASSIGNABLE_TO_TYPE_1_RE = exports.TYPE_NULL_IS_NOT_ASSIGNABLE_TO_TYPE_1_RE = /^Type '(.*)\bnull\b(.*)' is not assignable to type '.*'\.$/;
const TYPE_UNDEFINED_IS_NOT_ASSIGNABLE_TO_TYPE_1_RE = exports.TYPE_UNDEFINED_IS_NOT_ASSIGNABLE_TO_TYPE_1_RE = /^Type '(.*)\bundefined\b(.*)' is not assignable to type '.*'\.$/;
const ARGUMENT_OF_TYPE_0_IS_NOT_ASSIGNABLE_TO_PARAMETER_OF_TYPE_1_ERROR_CODE = exports.ARGUMENT_OF_TYPE_0_IS_NOT_ASSIGNABLE_TO_PARAMETER_OF_TYPE_1_ERROR_CODE = 2345;
const OBJECT_IS_POSSIBLY_UNDEFINED_ERROR_CODE = exports.OBJECT_IS_POSSIBLY_UNDEFINED_ERROR_CODE = 2532;
const NO_OVERLOAD_MATCHES_THIS_CALL_ERROR_CODE = exports.NO_OVERLOAD_MATCHES_THIS_CALL_ERROR_CODE = 2769;
const ARGUMENT_OF_TYPE_NULL_IS_NOT_ASSIGNABLE_TO_PARAMETER_OF_TYPE_1_RE = exports.ARGUMENT_OF_TYPE_NULL_IS_NOT_ASSIGNABLE_TO_PARAMETER_OF_TYPE_1_RE = /^Argument of type '(.*)\bnull\b(.*)' is not assignable to parameter of type '.*'\.$/;
const ARGUMENT_OF_TYPE_UNDEFINED_IS_NOT_ASSIGNABLE_TO_PARAMETER_OF_TYPE_1_RE = exports.ARGUMENT_OF_TYPE_UNDEFINED_IS_NOT_ASSIGNABLE_TO_PARAMETER_OF_TYPE_1_RE = /^Argument of type '(.*)\bundefined\b(.*)' is not assignable to parameter of type '.*'\.$/;
class LibraryTypeCallDiagnosticChecker {
  constructor(filteredDiagnosticMessages) {
    _defineProperty(this, "inLibCall", false);
    _defineProperty(this, "diagnosticMessages", void 0);
    _defineProperty(this, "filteredDiagnosticMessages", void 0);
    this.filteredDiagnosticMessages = filteredDiagnosticMessages;
  }
  configure(inLibCall, diagnosticMessages) {
    this.inLibCall = inLibCall;
    this.diagnosticMessages = diagnosticMessages;
  }
  checkMessageText(msg) {
    if (this.inLibCall) {
      const match = msg.match(ARGUMENT_OF_TYPE_NULL_IS_NOT_ASSIGNABLE_TO_PARAMETER_OF_TYPE_1_RE) || msg.match(ARGUMENT_OF_TYPE_UNDEFINED_IS_NOT_ASSIGNABLE_TO_PARAMETER_OF_TYPE_1_RE) || msg.match(TYPE_UNDEFINED_IS_NOT_ASSIGNABLE_TO_TYPE_1_RE) || msg.match(TYPE_NULL_IS_NOT_ASSIGNABLE_TO_TYPE_1_RE);
      return !match;
    }
    return true;
  }
  checkMessageChain(chain) {
    if (chain.code === TYPE_0_IS_NOT_ASSIGNABLE_TO_TYPE_1_ERROR_CODE) {
      if (chain.messageText.match(TYPE_UNKNOWN_IS_NOT_ASSIGNABLE_TO_TYPE_1_RE)) {
        return false;
      }
      if (this.inLibCall && chain.messageText.match(TYPE_UNDEFINED_IS_NOT_ASSIGNABLE_TO_TYPE_1_RE)) {
        return false;
      }
      if (this.inLibCall && chain.messageText.match(TYPE_NULL_IS_NOT_ASSIGNABLE_TO_TYPE_1_RE)) {
        return false;
      }
    }
    if (chain.code === ARGUMENT_OF_TYPE_0_IS_NOT_ASSIGNABLE_TO_PARAMETER_OF_TYPE_1_ERROR_CODE) {
      if (this.inLibCall && chain.messageText.match(ARGUMENT_OF_TYPE_UNDEFINED_IS_NOT_ASSIGNABLE_TO_PARAMETER_OF_TYPE_1_RE)) {
        return false;
      }
      if (this.inLibCall && chain.messageText.match(ARGUMENT_OF_TYPE_NULL_IS_NOT_ASSIGNABLE_TO_PARAMETER_OF_TYPE_1_RE)) {
        return false;
      }
    }
    return chain.next === undefined ? true : this.checkMessageChain(chain.next[0]);
  }
  checkFilteredDiagnosticMessages(msgText) {
    if (this.filteredDiagnosticMessages.size === 0) {
      return true;
    }
    if (typeof msgText !== 'string' && this.filteredDiagnosticMessages.has(msgText)) {
      return false;
    }
    for (const msgChain of this.filteredDiagnosticMessages) {
      if (typeof msgText == 'string') {
        if (msgText === msgChain.messageText) {
          return false;
        }
        continue;
      }
      let curMsg = msgText;
      let curFilteredMsg = msgChain;
      while (curMsg) {
        if (!curFilteredMsg) {
          return true;
        }
        if (curMsg.code !== curFilteredMsg.code) {
          return true;
        }
        if (curMsg.messageText !== curFilteredMsg.messageText) {
          return true;
        }
        curMsg = curMsg.next ? curMsg.next[0] : undefined;
        curFilteredMsg = curFilteredMsg.next ? curFilteredMsg.next[0] : undefined;
      }
      return false;
    }
    return true;
  }
  checkDiagnosticMessage(msgText) {
    if (!this.diagnosticMessages) {
      return false;
    }
    if (this.inLibCall && !this.checkFilteredDiagnosticMessages(msgText)) {
      return false;
    }
    if (typeof msgText == 'string') {
      return this.checkMessageText(msgText);
    }
    if (!this.checkMessageChain(msgText)) {
      this.diagnosticMessages.push(msgText);
      return false;
    }
    return true;
  }
}
exports.LibraryTypeCallDiagnosticChecker = LibraryTypeCallDiagnosticChecker;
//# sourceMappingURL=LibraryTypeCallDiagnosticChecker.js.map