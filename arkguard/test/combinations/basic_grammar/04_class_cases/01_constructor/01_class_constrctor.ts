import assert from "assert";
// 单个构造器
class A1 {
  prop1: number;
  constructor(para: number) {
    this.prop1 = para;
  }
  method1() {
  }
}
let insA1 = new A1(1);
assert(insA1.prop1 === 1)
// 多个构造器
class A2 {
  prop2: string;
  constructor(para: 'hello');
  constructor(para: 'bye');
  constructor(para: string);
  constructor(para: any) {
    this.prop2 = para;
  }
  method2() {
  }
}
let insA2 = new A2('bee');
assert(insA2.prop2 === 'bee')
// class声明 - 两个构造器不连续
declare namespace ns {
  class A3 {
    constructor();
    str3: string;
    constructor(x: number);
  }
}

declare class A4 {
  constructor();
  str4: string;
  constructor(x: number);
}