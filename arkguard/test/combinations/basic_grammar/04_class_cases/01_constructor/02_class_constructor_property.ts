import assert from "assert";
class A5 {
  prop_5 = 5;
  constructor(public para1: number, private para2: string, protected para3: boolean, readonly para4: number, para5: string) {
    para5 = para5 + 1;
    let temp1 = para1;
    let temp2 = para2;
    let temp3 = para3;
    let temp4 = para4;
    this.prop_5 = para4;
  }
}
let insA5 = new A5(1, '2', false, 4, '5');
assert(insA5.prop_5 === 4);
