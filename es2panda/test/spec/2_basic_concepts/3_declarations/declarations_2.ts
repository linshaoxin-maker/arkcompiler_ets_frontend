/**---
description: >
    Declarations introduce the following meanings for the name they declare:
    A variable, parameter, function, generator, member variable, member function, member accessor, or enum member declaration introduces a value meaning.
    An interface, type alias, or type parameter declaration introduces a type meaning.
    A class declaration introduces a value meaning (the constructor function) and a type meaning (the class type).
    An enum declaration introduces a value meaning (the enum instance) and a type meaning (the enum type).
    A namespace declaration introduces a namespace meaning (the type and namespace container) and, if the namespace is instantiated, a value meaning (the namespace instance).
    An import or export declaration introduces the meaning(s) of the imported or exported entity.
 ---*/

//class declaration
class C {
    x: string;
    constructor(x: string) {
        this.x = x;
    }
}
var c: C = new C("x");
Assert.equal(c, "x");

//enum declaration
enum WeekDay {
    MON = 1,
    TUE,
    WEN,
    THU,
    FRI,
    SAT,
    SUN
}
type a = WeekDay;
var mon: a = WeekDay.MON;
Assert.equal(mon, 1);

//namespace declaration
namespace N {
    export var x: string = "x";
}
Assert.equal(N.x, "x");
