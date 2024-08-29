import assert from 'assert';

const var1: string = "A";
assert.strictEqual(var1, "A");

try {
  // @ts-expect-error
  var1 = "B";
} catch (e) {
  assert.strictEqual((e as TypeError).name, "TypeError");
  assert.strictEqual((e as TypeError).message, "Assignment to constant variable.");
}

const var2: { name1: string, version1: string } = {
  name1: "arkguard",
  version1: "1.0.0"
}

assert.strictEqual(var2.name1, "arkguard")
assert.strictEqual(var2.version1, "1.0.0");

try {
  // @ts-expect-error
  var2 = "another";
} catch (e) {
  assert.strictEqual((e as TypeError).name, "TypeError");
  assert.strictEqual((e as TypeError).message, "Assignment to constant variable.");
}

var2.version1 = "1.0.1"
assert.strictEqual(var2.name1, "arkguard")
assert.strictEqual(var2.version1, "1.0.1");

const {name1: name2, version1: version2}: {name1: string, version1: string} = var2;
assert.strictEqual(name2, "arkguard");
assert.strictEqual(version2, "1.0.1");