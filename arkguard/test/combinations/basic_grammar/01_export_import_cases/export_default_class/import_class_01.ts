import assert from 'assert';
import class01 from './export_class_01'
import class02 from './export_class_02'
let ins1 = new class01('lion');
let ins2 = new class02('tiger');
assert(ins1.mName === 'lion');
assert(ins2.nName === 'tiger');
