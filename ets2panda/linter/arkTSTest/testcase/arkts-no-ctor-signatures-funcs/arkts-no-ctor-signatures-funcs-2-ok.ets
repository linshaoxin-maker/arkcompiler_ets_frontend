/*
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


class Car {
  public brand: string;
  public model: string;
  public year: number;

  constructor(brand: string, model: string, year: number) {
    this.brand = brand;
    this.model = model;
    this.year = year;
  }

  startEngine(): void {
    console.log(`Starting the engine of ${this.brand} ${this.model}`);
  }
}

type carCtor = (brand: string, model: string, year: number) => Car;

function create(ctor: carCtor, brand: string, model: string, year: number): Car {
  return ctor(brand, model, year);
}

const carCreator: carCtor = (brand: string, model: string, year: number): Car => {
  return new Car(brand, model, year);
}

const car = create(carCreator, 'BYD', 'han', 2022);
console.log(car.brand); // BYD
console.log(car.model); // han
console.log(car.year); // 2022
car.startEngine(); // Starting the engine of BYD han