import assert from 'assert'
import defaultApi from './export_namespaces_default'
import { unDefaultApi } from './export_namespaces_default'
let ins = new defaultApi.Person(2)
assert(ins.doubleProp1() === 4)
assert(unDefaultApi() === 3);
