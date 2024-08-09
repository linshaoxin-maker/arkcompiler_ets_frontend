import assert from 'assert'
import default_01 from './export_default_01_conditional'
assert(default_01 === 2);
import default_02 from './export_default_02_instance'
assert(default_02.num === 1)
import default_03 from './export_default_03_this'
assert(default_03 === undefined)
import default_04 from './export_default_04_var'
assert(default_04 === 4)
import default_05 from './export_default_05_array'
assert(default_05[0] === 1)
assert(default_05[1] === 2)
import default_06 from './export_default_06_object'
assert(default_06.objProp1 === 1)
assert(default_06.objProp2 === 2)
import default_07 from './export_default_07_regex'
assert(default_07.toString() === '/a/')
import default_08 from './export_default_08_str1'
assert(default_08 === `str1`)
import default_09 from './export_default_09_str2'
assert(default_09 === 'str2')
import default_10 from './export_default_10_str3'
assert(default_10 === "str3str4")
import default_11 from './export_default_11_conditional'
assert(default_11 === 2)
import default_12 from './export_default_12_meta'
assert(default_12)  //  xxxx
import default_13 from './export_default_13_module'
assert(default_13.num === 1)
import default_14 from './export_default_14_namepsace'
assert(default_14.num === 1)
import default_15 from './export_default_15_object'
assert(default_15.num === 1)
import default_16 from './export_default_16_function_ans'
assert(default_16.num === 1)
import default_17 from './export_default_17_function_conditional'
assert(default_17.num === 1)
import default_18 from './export_default_18_arrow'
assert(default_18.num === 1)
import default_19 from './export_default_19_arrow_async'
assert(default_19.num === 1)
import default_20 from './export_default_20_expression_01'
assert(default_20.num === 1)
import default_21 from './export_default_21_expression_02'
assert(default_21.num === 1)
import default_22 from './export_default_22_expression_03'
assert(default_22.num === 1)
import default_23 from './export_default_23_expression_04'
assert(default_23.num === 1)
import default_24 from './export_default_24_expression_05'
assert(default_24.num === 1)
import default_25 from './export_default_25_object'
assert(default_25.num === 1)
