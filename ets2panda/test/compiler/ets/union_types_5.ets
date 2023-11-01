class A<T extends Object> {
    union0: T;
}
class B {
    num: int = 42
}
class C {
    num: int = 43
}

function foo(x: B|C) {
    if (x instanceof B) {
        assert x.num == 42: "Error! num field must be 42";
    } else if (x instanceof C) {
        assert x.num == 43: "Error! num field must be 43";
    } else {
        assert false: "Error! x must be instanceof B|C";
    }
}

function bar(x: A<B|C>) {
    if (x.union0 instanceof B) {
        assert x.union0.num == 42: "Error! x.union0.num field must be 42";
    } else if (x.union0 instanceof C) {
        assert x.union0.num == 43: "Error! x.union0.num field must be 43";
    } else {
        assert false: "Error! x.union0 must be instanceof B|C";
    }
}

function main(): void {
    let a: A<B|C> = new A<B|C>();
    a.union0 = new B();
    foo(a.union0);
    bar(a);
    a.union0 = new C();
    bar(a);
    foo(a.union0);
}
