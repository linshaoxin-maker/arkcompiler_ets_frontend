import assert from 'assert'

interface Foo {
    method(a: number): string;
    optionalMethod?(a: number): string;
    property: string;
    optionalProperty: string;
}
  
class Foo {
    additionalProperty: string = '';
    additionalMethod(a: number): string {
      return this.method(0);
    }
}
class Bar extends Foo {
    method(a: number): string {
      return this.optionalProperty;
    }
}

let a = new Bar();
assert(a.method(1) === undefined)
assert(a.optionalMethod === undefined)
assert(a.property === undefined)
assert(a.optionalProperty === undefined)
assert(a.additionalProperty === '')
assert(a.additionalMethod(1) === undefined)