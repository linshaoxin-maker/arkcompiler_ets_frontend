import assert from 'assert'

export type callback<T> = ()=>T;
export type CallbackArray<T extends callback<T>> = ()=>T
type t = ()=>t;
let a: CallbackArray<()=>t>
a = ()=>a;
assert(a() === a);

let var1: number = 1;
typeof var1;
type t01 = typeof var1;
let a2:t01 = 1;
assert(a2 === 1);

let c: [string, number, boolean] = ["", 1, false]
assert(c === ["", 1, false])

type a = [number, string, ...number[]]

let temp1: number | string = 1;
assert(temp1 === 1);
let temp2: number & (string | number) = 1;
assert(temp2 === undefined)
type temp7 = number;
type temp8 = string;
function foo<T>(param: T extends temp7 ? temp7 : temp8) { return param}
assert(foo<number>(1) === 1)

type X2<T> = T extends {a: infer U, b: infer U} ? U : never;
let x: X2<{a:number, b:number}> = 1
assert(x === 1)

let temp6: (string | number)[] = [1,2];
assert(temp6 === [1,2])

interface Person {
  name: string;
  age: number;
}
type PersonKeys = keyof Person;
let b: PersonKeys = "name"
assert(b === "name")


type T1 = {U:number};
let temp5:T1['U'] = 2;
assert(temp5 === 2)

type Foo<T extends any[]> = {
  [P in keyof T]: T[P]
}
let d:Foo<number[]> = [1];
assert(d === [1])

let temp3: "cc" = "cc"
assert(temp3 === "cc");
let temp4: [prop1: string, prop2: number] = ["1",2];
assert(temp4 === ["1", 2]);
