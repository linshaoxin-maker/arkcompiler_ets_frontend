import assert from 'assert' 

let s1:symbol = Symbol();
let s2 = Symbol("s")
const s3:unique symbol = Symbol("s3")

let a = {
  [s1]:1
};

a[s1];
assert(a[s1] === 1);

//Symbol.asyncIterator;
Symbol.hasInstance
Symbol.isConcatSpreadable
Symbol.iterator
Symbol.match
Symbol.replace
Symbol.search
Symbol.species
Symbol.split
Symbol.toPrimitive
Symbol.toStringTag
Symbol.unscopables

let o1 = {
  //[Symbol.asyncIterator]:1,
  [Symbol.hasInstance]:2,
  [Symbol.isConcatSpreadable]:3,
  [Symbol.iterator]:4,
  [Symbol.match]:5,
  [Symbol.replace]:6,
  [Symbol.search]:7,
  [Symbol.species]:8,
  [Symbol.split]:9,
  [Symbol.toPrimitive]:10,
  [Symbol.toStringTag]:11,
  [Symbol.unscopables]:12,
}

//assert(o1[Symbol.asyncIterator] === 1);
assert(o1[Symbol.hasInstance] === 2);
assert(o1[Symbol.isConcatSpreadable] === 3);
assert(o1[Symbol.iterator] === 4);
assert(o1[Symbol.match] === 5);
assert(o1[Symbol.replace] === 6);
assert(o1[Symbol.search] === 7);
assert(o1[Symbol.species] === 8);
assert(o1[Symbol.split] === 9);
assert(o1[Symbol.toPrimitive] === 10);
assert(o1[Symbol.toStringTag] === 11);
assert(o1[Symbol.unscopables] === 12);