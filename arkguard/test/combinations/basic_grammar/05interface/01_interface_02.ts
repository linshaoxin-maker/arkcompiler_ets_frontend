import assert from 'assert'

interface Generic<T> {
  x:T
}
var y: Generic<number> = {x:3};

assert(y.x === 3)