import assert from 'assert'

//array
let a=1;
let arr = [0,a,3];
assert(a === 1)
assert(arr[0] === 0)
assert(arr[1] === 1)
assert(arr[2] === 3)
let [b,c,d] = arr;
assert(b === 0);
assert(c === 1);
assert(d === 3);
[b,,c] = [...arr];
assert(b === 0);
assert(c === 3);

//object
let obj = {a:1, b:a+1, c:3}
assert(obj.a === 1);
assert(obj.b === 2);
assert(obj.c === 3);
let {c:b2,a:c2,b:d2}=obj;
assert(b2 === 3);
assert(c2 === 1);
assert(d2 === 2);

class C1 {
  a:number;
  b:number;
  c:number;
  constructor([_a,_b,_c]:Array<number>) {
    this.a = _a;
    this.b = _b;
    this.c = _c;
  }
}
function f1([a,b,c]: Array<number>, {a:x,b:y,c:d}:C1) {
  a;b;c;x;y;d;
  assert(a === 10);
  assert(b === 21);
  assert(c === 20);
  assert(x === 0);
  assert(y === 1);
  assert(d === 3);
}
f1([10,a+20,20],new C1([...arr]))