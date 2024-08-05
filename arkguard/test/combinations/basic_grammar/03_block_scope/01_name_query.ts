import assert from "assert";
// 名称访问
{
  let a1 = 1;
  let a2 = 2;
  let a3 ;
  a3 = a1 + a2;
  assert(a3 === 3)
}
// 外部名称访问
let out_var1 = 1;
{
  let var2 = 2;
  let var3;
  var3 = var2 + out_var1;
  assert(var3 === 3)
}
// 属性访问
{
  class p {
    prop1: number = 1;
  }
  let q = new p();
  q.prop1 = 2;
  assert(q.prop1 === 2)
}
// var变量
var x = 1; {
  var x = 2;
}
assert(x === 2);

// 函数
{
  foo()
  function foo(){
    return 'foo'
  }
  assert(foo() === 'foo');
}
// assert(foo()); undefined, 检查异常

// 块语句封装数据
let rectangle;
{
  const width = 2;
  const length = 10;
  rectangle = {
    width,
    length,
    area: width * length,
    perimeter: (width + length) * 2
  }
}
assert(rectangle.area === 20);
assert(rectangle.perimeter === 24);
