/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

enum Color {
  RED = 1,
  GREEN = 2,
  BLUE = 4
}

namespace Color {
  export function mixColor(colorName: string): any {
    if (colorName === "yellow") {
      return Color.RED + Color.GREEN;
    } else if (colorName === "white") {
      return Color.RED + Color.GREEN + Color.BLUE;
    } else if (colorName === "magenta") {
      return Color.RED + Color.BLUE;
    } else if (colorName === "cyan") {
      return Color.GREEN + Color.BLUE;
    }
  }
}

print(Color.mixColor('yellow') === 3, 'success');

print(Color.mixColor('white') === 7, 'success');

print(Color.mixColor('magenta') === 5, 'success');

print(Color.mixColor('cyan') === 6, 'success');
