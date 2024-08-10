import assert from 'assert'

const s:unique symbol = Symbol();
class C {
  a?: number;
  b?: string;
  c?: boolean;
  1?: number;
  "x"?: number;
  [s]?: number;
}

let x = 10;
let y = {a:20};
let s2:symbol;
namespace z {
  export let a = 30;
}
let a = {
  a:1,
  b:"12",
  1:1,
  "2":"2",
  [3]:3,
  ["4"]:"4",
  [s2 = Symbol()]:5,
  [s]:6,
  ["1"+"2"]:7,
  [1+2+3]:8,
  [x]:9,
  [x+3]:10,
  [y.a]:11,
  [z.a]:12,
  x:()=>{ return 13},
  await:()=>14
}
assert(a.a === 1);
assert(a.b === '12');
assert(a[1] === 1);
assert(a["2"] === "2");
assert(a[3] === 3);
assert(a["4"] === "4");
assert(a[s2] === 5);
assert(a[s] === 6);
assert(a["12"] === 7);
assert(a[6] === 8);
assert(a[10] === 9);
assert(a[13] === 10);
assert(a[20] === 11);
assert(a[30] === 12);
assert(a.x() === 13);
assert(a.await() === 14);


function g({b,...a}:{b:number, c:string}) {
  return b+a.c;
}
g({b:1, c:"213"});
assert(g({b:1, c:"213"}) === "1213")

function f(x:C) {
  assert(x.a === 1);
  assert(x.b === "1");
  assert(x.c === true);
  assert(x[1] === 1);
  assert(x["x"] === 2);
  assert(x[s] === 3);
}
f({a:1, b:"1", c:true, 1:1, "x":2, [s]:3})
