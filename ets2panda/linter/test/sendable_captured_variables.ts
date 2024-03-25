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


import { hfpProfileSwitchMenuStyle } from "./object_literals.ts"

let local = 'local';

@Sendable
class SendableClass {
	public p = local; // error
    static ps = local; // error
	public foo(x: string): string {
		let s = local;  //error
    		let ss = this.p + x;
            return s + ss;
	}
	static {
		SendableClass.ps = local; //error
		let pps = hfpProfileSwitchMenuStyle;
	}

}
