"use strict";

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.LoggerImpl = void 0;
var _logger = _interopRequireDefault(require("../utils/logger"));
function _interopRequireDefault(e) { return e && e.__esModule ? e : { default: e }; }
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

class LoggerImpl {
  trace(message) {
    void this;
    _logger.default.getLogger().trace(message);
  }
  debug(message) {
    void this;
    _logger.default.getLogger().debug(message);
  }
  info(message) {
    void this;
    _logger.default.getLogger().info(message);
  }
  warn(message) {
    void this;
    _logger.default.getLogger().warn(message);
  }
  error(message) {
    void this;
    _logger.default.getLogger().error(message);
  }
}
exports.LoggerImpl = LoggerImpl;
//# sourceMappingURL=LoggerImpl.js.map