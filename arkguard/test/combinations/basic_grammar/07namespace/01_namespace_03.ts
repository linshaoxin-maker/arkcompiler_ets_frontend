import assert from 'assert'

declare global {

}
declare global {
  export module global {
    let val: number;
    type t = number;
  }
}
let a:global.t = 1;
assert(a===1);

namespace ns {
  export module a {
    export function foo() {
      return 1;
    }
  }
  module b {

  }
}
assert(ns.a.foo() === 1);

export {}