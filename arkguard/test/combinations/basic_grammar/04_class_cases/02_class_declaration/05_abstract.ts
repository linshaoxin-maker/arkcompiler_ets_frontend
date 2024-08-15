import assert from "assert";
abstract class abstractC1 {
  abstract methodAC1(): number;
  methodAc2(): string {
    return '22';
  }
}

class C8 extends abstractC1 {
  methodAC1(): number {
    return 11;
  }
}
let insC8 = new C8();
assert(insC8.methodAC1() === 11, 'success');
assert(insC8.methodAc2() === '22', 'success');


class C9 extends abstractC1 {
  methodAC1(): number {
    return 111;
  }
  methodAc2(): string {
    return '222';
  }
}
let insC9 = new C9();
assert(insC9.methodAC1() === 111, 'success');
assert(insC9.methodAc2() === '222', 'success');