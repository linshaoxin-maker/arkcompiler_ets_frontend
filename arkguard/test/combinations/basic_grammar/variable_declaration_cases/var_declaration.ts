import assert from 'assert';

// top-level var
var variable1: number = 1;
assert.strictEqual(variable1, 1);
// var be can defined multi times
var variable1: number = 2;
assert.strictEqual(variable1, 2);

function function1(): () => string {
  // function scope var
  var variable2: string = "hello";
  assert.strictEqual(variable2 + " world", "hello world");
  return function function2(): string {
    // lexical var
    var variable3: string = variable2 + "!";
    return variable3;
  }
}

assert.strictEqual(function1()(), "hello!");

var variable4: () => string = function1();
assert.strictEqual(variable4(), "hello!");

function function3(): number {
  var variable5: number = 1;
  variable5 = 2;
  // function declaration hoisting
  var variable6: number = function4();
  assert.strictEqual(variable6, 2);
  variable5 = 3;
  assert.strictEqual(variable5, 3);
  return variable6;

  function function4(): number {
    return variable5;
  }
}

assert.strictEqual(function3(), 2);

function function5(flag: boolean): number | undefined {
  if (flag) {
    var variable7: number = 10;
  }
  // var hoisting
  return variable7;
}

assert.strictEqual(function5(true), 10);
assert.strictEqual(function5(false), undefined);
