import assert from 'assert'

enum A {
    prop1 = 1,
    prop2 = "2",
}
A.prop1
A.prop2

assert(A.prop1 === 1);
assert(A.prop2 === "2");
assert(A[1] === 'prop1');

const enum B {
    prop1 = 1,
    prop2 = "2",
}
B.prop1
B.prop2

assert(B.prop1 === 1);
assert(B.prop2 === "2");