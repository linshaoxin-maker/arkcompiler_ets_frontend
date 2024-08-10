import assert from 'assert'

var v1 = function f() {
    let v1 = 'test';
    return v1;
}
v1();
assert(v1() === 'test');

var v2 = {
    c: function () {
        return v2;
    },
    d: function f() {
        return v2.c;
    }
}
v2.c();
v2.d();
assert(v2.c() === v2);
assert(v2.d()() === v2);


const {
    B = function () {
      let B = 'binding'
    },
    C = function g() {
      let C = 'binding'
      return C;
    }
} = {B: undefined, C: ()=>{
  return 'test';
}}
B;
C();
assert(B === undefined);
assert(C() === 'test');

var x = function g() {
  return g;
}
var y = function g() {
  return y;
}
x();
y()();
assert(x() === x);
assert(y()() === y);

var z = function f(para: any, ...paras:any):any {
  return arguments;
}
z(1,2,3)
assert(z(1,2,3) === {"0": 1, "1": 2, "2": 3})