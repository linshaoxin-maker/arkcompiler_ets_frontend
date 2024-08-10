import assert from 'assert'

class Person {
  name: string = "";
  set personName(name: string) {
    this.name = name;
  }
}

let p = new Person();
p.personName === "name"

assert(p.name === "name")