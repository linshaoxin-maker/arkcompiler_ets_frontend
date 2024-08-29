import assert from 'assert'

let name1 = 'hello'
let info1 = {name1}
info1.name1
assert(name1 === 'hello');
assert(info1.name1 === 'hello');

let name2 = 'hello'
let info2 = {name: name1}
info2.name
assert(name2 === 'hello');
assert(info2.name === 'hello');