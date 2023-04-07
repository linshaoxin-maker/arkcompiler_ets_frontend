/**---
description: >
    A name that denotes a value has an associated type and can be referenced in expressions. 
    A name that denotes a type can be used by itself in a type reference or on the right hand side of a dot in a type reference. 
    A name that denotes a namespace can be used on the left hand side of a dot in a type reference.
 ---*/

//name denotes a value
var X: string = "X is string";
Assert.equal("X is string1", X + "1");

//name denotes a type
type X = String | Number;
var x: X = "xx";
Assert.equal("xx", x);

//name denotes a namespace
namespace X {
    export type Y = string;
}
var y: X.Y = "ystr";
Assert.equal("ystr", y);