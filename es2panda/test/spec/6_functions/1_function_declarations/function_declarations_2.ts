/**---
description: >
    Function declarations are extended to permit the function body to be omitted in overload declarations.
    a function can have at most one implementation.

    When a function has both overloads and an implementation, the overloads must precede the implementation 
    and all of the declarations must be consecutive with no intervening grammatical elements.
 ---*/

function add(a: number, b: number): number;
function add(a: string, b: number): string;
function add(a: string, b: string): string;
function add(arg1: string | number, arg2: string | number) {
    if (typeof arg1 === "number" && typeof arg2 === "number") {
        return arg1 + arg2;
    }

    if (typeof arg1 === "string" || typeof arg2 === "string") {
        return `${arg1}${arg2}`;
    }
}
// (number,number)
Assert.equal(add(0, 1), 1);
// (string,number)
Assert.equal(add("0", 1), "01");
// (string,string)
Assert.equal(add("0", "1"), "01");
