/**---
 description: >
    A property name can be any identifier (including a reserved word), a string literal, a numeric literal, 
    or a computed property name. String literals may be used to give properties names that are not valid identifiers, 
    such as names containing blanks. Numeric literal property names are equivalent to string literal property names 
    with the string representation of the numeric literal, as defined in the ECMAScript specification.
 ---*/

class Property {
    //reserved word as property name
    break: string;
    //string literal as property name
    "with blank": number;
    //numeric literal as property name
    1: boolean;
    constructor(x: string, y: number, z: boolean) {
        this.break = x;
        this["with blank"] = y;
        this[1] = z;
    }
}

var p: Property = new Property("abc", 12, false);
Assert.equal("abc", p.break);
Assert.equal(12, p["with blank"]);
Assert.equal(false, p[1]);