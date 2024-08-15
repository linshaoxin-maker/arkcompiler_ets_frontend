import assert from "assert";
// 单实现interface
interface I1 {
  prop1: number;
}
class A1 implements I1 {
  prop1: number = 2;
}
let insA1 = new A1();
assert(insA1.prop1 === 2)

// 多实现interface
interface I2 {
  fly(): string;
}

interface I3<T> {
  swim(): void;
}
class A2<U> implements I2, I3<number> {
  swim(): void {
    let tempVal = 'i can swim';
    return;
  }
  fly<U>(): string {
    return 'i can fly';
  }
  prop2: number;
}
let insA2 = new A2();
assert(insA2.fly() === 'i can fly')
// 单实现class
class C1 {
  method1() { return 4}
}
class C2 implements C1 {
  method1(): number {
    return 1
  }
}
let insC2 = new C2();
assert(insC2.method1() === 1)
// 多实现class
class C3 {
  prop3: number;
  method3(): number {
    return 1;
  }
}

class C4 {
  method4(para: string): string {
    return 'hello 4' + para
  }
}

class C5 implements C3, C4 {
  method4(para: string): string {
    return 'hello 44' + para;
  }
  prop3: number;
  method3(): number {
    return 33;
  }
}
let insC5 = new C5();
assert(insC5.method3() === 33)
assert(insC5.method4('55') === 'hello 4455')