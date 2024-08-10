import assert from 'assert'
import typeAlias4, {typeAlias1, type typeAlias2} from './export_type'
import type { typeAlias3} from './export_type'
import type {default as typeAlias_new4 } from './export_type'
import type { as } from './export_type'
import * as module2 from './export_type'
import * as type from './export_type'

let num1: typeAlias1 = 1;
let num2: typeAlias2 = 'blank';
let num3: typeAlias3 = false;
let num4: typeAlias_new4 = 4;
let num5: as;
let num6: module2.typeAlias1 = 6;
let num7: type.typeAlias1 = 7;

