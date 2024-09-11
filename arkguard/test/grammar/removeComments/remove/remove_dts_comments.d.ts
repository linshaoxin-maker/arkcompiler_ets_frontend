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

/**
 * A global string variable.
 */
declare const globalVar: string;

/**
 * Enumeration of options.
 */
declare enum MyEnum {
  /**
   * Enum properties
   */
  FIRST,
  SECOND,
  THIRD
}

/**
 * Interface defining the structure of an object.
 */
declare interface MyInterface {
  /** A string property */
  prop1: string;
  
  /** A number property */
  prop2: number;
  
  /**
   * A method that takes a string parameter.
   * @param param A string parameter
   */
  method1(param: string): void;
}

/**
 * Class implementing MyInterface.
 */
declare class MyClass implements MyInterface {
  /** A string property */
  prop1: string;
  
  /** A number property */
  prop2: number;

  /**
   * Constructor to initialize the class.
   * @param prop1 A string parameter
   * @param prop2 A number parameter
   */
  constructor(prop1: string, prop2: number);
  
  /**
   * A method that takes a string parameter.
   * @param param A string parameter
   */
  method1(param: string): void;

  /**
   * A method that takes a number parameter.
   * @param param A number parameter
   */
  method2(param: number): boolean;

  /**
   * A static method of the class.
   */
  static staticMethod(): void;
}

/**
 * Function that takes an enum value and returns an interface.
 * @param param A value of MyEnum
 * @returns An object implementing MyInterface
 */
declare function dtsFunction(param: MyEnum): MyInterface;

/**
 * A namespace containing related variables and functions.
 */
declare namespace MyNamespace {
  /** A string variable inside the namespace */
  const nsVar: string;
}

/** 
 * This is a module declaration.
 * Declares a module with an exported function.
 */
declare module "my-module" {
  export function moduleFunction(): void;
}

/**
 * A type containing a property
 */
declare type testType = {
  /**
   * type property
   */
  name: string;
};
