import assert from 'assert';

let value1 = 1;
assert.strictEqual(value1, 1);

function func1() {
  return value1;
}
assert.strictEqual(func1(), 1);


function func2(flag) {
  let value2 = 2;
  {
    let value3 = 3;
    assert.strictEqual(value3, 3);
  }
  if (flag) {
    return value2;
  } else {
    return value3;
  }
}

assert.strictEqual(func2(true), 2);

try {
  func2(false);
} catch (e) {
  assert.strictEqual(e.name, "ReferenceError");
  assert.strictEqual(e.message, "value3 is not defined");
}

function func3(condition, value4) {
  if (condition) {
    let value4 = 100;
    return value4;
  }

  return value4;
}

assert.strictEqual(func3(false, 0), 0);
assert.strictEqual(func3(true, 0), 100);

function func4() {
  let func5;

  if (true) {
      let message = "hello";
      func5 = function() {
          return message;
      }
  }

  return func5();
}

assert.strictEqual(func4(), "hello");