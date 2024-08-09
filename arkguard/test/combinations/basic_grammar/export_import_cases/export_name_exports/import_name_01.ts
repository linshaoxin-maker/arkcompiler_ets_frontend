import assert from 'assert'
import {var1, var2, foo1 ,foo2, Person1} from './export_name_01'

assert(var1 === 1);
assert(var2 === 2);
assert(foo1() === 11);
assert(foo2() === 22);
let ins = new Person1();
assert(ins.getAge === 1)