import assert from 'assert'

let x:number = 1;
let y:string = "1";
let z:boolean  = true;

let classNum = 0;
let methosNum = 0;
let PropertyNum = 0;
let f1:ClassDecorator = () => {classNum++;};
let f2:MethodDecorator = () => {methosNum++;};;
let f3:PropertyDecorator = () => {PropertyNum++;};;

function f11(a:number):ClassDecorator {assert(a === 1); return f1};
function f21(b:string):MethodDecorator {assert(b === "1");return f2};
function f31(c:boolean):PropertyDecorator {assert(c === true);return f3};

type f1 = number;
type f2 = string;
type f3 = boolean;

@f1
@f11(x)
class C{
  @f2
  @f21(y)
  f(){}
  @f2
  @f21(y)
  get(){}
  @f21(y)
  @f2
  set(){}
  @f3
  @f31(z)
  a:number=1;
}
assert(classNum === 2);
assert(methosNum === 6);
assert(PropertyNum === 2);