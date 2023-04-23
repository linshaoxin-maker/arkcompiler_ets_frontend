/**---
description: >
    ECMAScript 2015 permits object literals and classes to declare members with computed property names. 
    A computed property name specifies an expression that computes the actual property name at run-time.
options：
    target：es2015
 ---*/

class ComputedName {
    //computed property name with a well-known symbol name  have a simple literal type
    [Symbol.toStringTag]: number;
    //computed property name have a simple literal type
    ["address"]: string;
    constructor(x: string, y: number) {
        this[Symbol.toStringTag] = y;
        this.address = x;
    }
}
var c: ComputedName = new ComputedName("address No1", 12);
Assert.equal(12, c[Symbol.toStringTag]);
Assert.equal("address No1", c.address);

//computed property name with in object literal
var objectliteral = { ["xx" + "123".length]: 22, name: "string" };
Assert.equal(22, objectliteral.xx3);