import assert from 'assert' 

class c{
  a:number = 1;
}
async function f():Promise<c>{return new c();}
async function g(){
  let a = await f();
  assert(a.a === 1);
}
g();