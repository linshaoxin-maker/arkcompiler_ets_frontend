import assert from 'assert'
import defaultApi1 from './export_api'

import * as module2 from './export_api'
import { } from './export_api'
import { var1 } from './export_api'
import { var2, var3, } from './export_api'
import { default as maxFunc1 } from './export_api'
import maxFunc2, * as module3 from './export_api'
import maxFunc3, { reduceFunc, } from './export_api'

assert(defaultApi1(2, 3) === 3);
assert(module2.var1 === 1);
assert(var1 === 1);
assert(var2 === 2);
assert(var3 === 1);
assert(maxFunc1(3, 4) === 4);
assert(maxFunc2(7, 8) === 8);
assert(module3.var3 === 3);
assert(maxFunc3(3, 4) === 4);
assert(reduceFunc(5, 4) === 1);

import "";
import ''
import './export_api'

import { moduleAlias } from './export_api_02'
import * as moduleAlias2 from './export_api_02'
assert(moduleAlias.addFunc(3, 4) === 7);
assert(moduleAlias2.moduleAlias.reduceFunc(5, 4) === 1);