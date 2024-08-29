import assert from 'assert'

module X {
  export module Y {
    export interface Z {
      a:number;
    }
  }
  export interface Y {
    b:string;
  }
}
let a:X.Y.Z = {a:1}
assert(a.a === 1);
let b:X.Y = {b:"1"}
assert(b.b === "1");
  
module A {
  export module B {
    export class C {
      c: boolean = true;
    }
  }
}
  
var c: A.B.C = new A.B.C();
assert(c.c === true)
  
module M {
  export namespace N {
    export module M2 {
      export interface I {
        d: number;
      }
    }
  }
}
let d: M.N.M2.I = {d:2}
assert(d.d === 2)

type A = number;
declare const Q1:number;
declare namespace Q2 {
  export {A}
}
let e:Q2.A = 3;
assert(e ===3);
  
export {}