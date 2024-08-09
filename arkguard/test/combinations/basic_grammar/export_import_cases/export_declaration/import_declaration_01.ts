import assert from "assert"
import { foo1, foo2, foo3, foo4 } from './export_declaration_01'
import { className1 } from './export_declaration_01'
import { var1, var2, var3, var4, var5, var6 } from './export_declaration_01'

assert(foo1() === 1);
assert(foo2().next().value === 1);
assert(foo2().next().value === 2);
assert(await foo3() === 3);
assert((await foo4().next()).value === 4);

let ins = new className1();
assert(ins.prop === 'hello');

assert(var1 === 1);
assert(var2 === 2);
assert(var3 === 3);
assert(var4 === 4);
assert(var5 === 5);
assert(var6 === 6);
