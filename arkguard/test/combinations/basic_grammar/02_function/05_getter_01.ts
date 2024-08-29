import assert from 'assert';

class Person {
  name: string = "name";
  get personName() {
    return this.name;
  }
}

let p = new Person();

assert(p.personName === "name");