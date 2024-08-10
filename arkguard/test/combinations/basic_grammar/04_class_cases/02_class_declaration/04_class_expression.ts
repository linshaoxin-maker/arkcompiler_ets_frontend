import assert from "assert";

interface I6 {
  prop_i6: number;
  method_i6(para: number): number;
}
let cons1 = class C7 implements I6 {
  prop_i6: number = 7;
  method_i6(para: number): number {
    return para * 2 * this.prop_i6;
  }
}
let insC7 = new cons1();
assert(insC7.prop_i6 === 7);
assert(insC7.method_i6(2) === 28);
