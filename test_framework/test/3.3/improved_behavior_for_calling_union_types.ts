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
   Improved behavior for calling union types.
 ---*/


{
    type Fruit = "apple" | "orange";
    type Color = "red" | "orange";
    // eats and ranks the fruit
    type FruitEater = (fruit: Fruit) => number;
    // eats and ranks the fruit
    let s: FruitEater = (fruit: Fruit) => 1;
    // consumes and describes the colors
    type ColorConsumer = (color: Color) => string;
    // consumes and describes the colors
    let sp: ColorConsumer = (color: Color) => 'good';

    Assert.equal(s("apple"), 1)
    Assert.equal(sp("orange"), 'good')

    interface Dog {
        kind: "dog";
        dogProp: any;
    }

    interface Cat {
        kind: "cat";
        catProp: any;
    }

    const catOrDogArray: Dog[] | Cat[] = [{ kind: "dog", dogProp: 1 }, { kind: "dog", dogProp: 2 }];
    let ex: number[] = [];
    catOrDogArray.forEach((animal: Dog | Cat) => {
        if (animal.kind === "dog") {
            ex.push(animal.dogProp);
        } else if (animal.kind === "cat") {
            ex.push(animal.catProp);
        }
    });

    Assert.equal(ex[0], 1)
    Assert.equal(ex[1], 2)
}
