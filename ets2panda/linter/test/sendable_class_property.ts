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

class NonSendableClass2 {}

@Sendable
class SendableClass4<T, U> {
  prop1: number; // OK
  prop2: string; // OK
  prop3: boolean; // OK
  prop4: bigint; // OK
  prop5: SendableClass3; // OK
  prop6: null; // OK
  prop7: undefined; // OK
  prop8: U; // OK
  prop9: T | null | undefined; // OK
}

@Sendable
class SendableClass3 {
  prop1: string[]; // ERROR, sendable class property cannot be array
  prop2: NonSendableClass2; // ERROR, sendable class property cannot be non-sendable-class
  prop3: NonSendableClass2 | null; // ERROR, sendable class property cannot be non-sendable-class union type
  prop4: NonSendableClass2 | undefined; // ERROR, sendable class property cannot be non-sendable-class union type
  prop5: NonSendableClass2 | null | undefined; // ERROR, sendable class property cannot be non-sendable-class union type
}