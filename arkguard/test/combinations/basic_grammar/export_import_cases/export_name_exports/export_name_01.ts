let var1 = 1;
let var2 = 2;
function foo1() {
  return 11;
}
function foo2(){
  return 22;
}
class Person1 {
  age:number = 1;
  get getAge(): number {
    return this.age;
  }
}
export {}
export {var1};
export {var2, foo1};
export {foo2, Person1,}

