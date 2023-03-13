/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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


class C {
    firstName: string;
    lastName: string;

    constructor(firstName: string, lastName: string) {
        this.firstName = firstName;
        this.lastName = lastName;
    }

    get fullName () {
        return 'get ' + this.firstName + '-' + this.lastName;
    }

    set fullName (val) {
        let names = val.split('-');
        this.firstName = names[0];
        this.lastName = names[1];
        print("set name success");
    }

    set<T> (a:T) {
        print(`common set ${a}`);
    }

    get<T> () {
        print(`common get function`);
    }
}

var c1 = new C("", "");
c1.set<string>("str");
c1.get<string>();
c1.fullName = "first-last";
print(c1.fullName);
