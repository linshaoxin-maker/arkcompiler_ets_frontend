class A {
    foo() {
        return 1;
    }

    static foo() {
        return 2;
    }
}

class B {
    static foo() {
        return 4;
    }

    foo() {
        return 3;
    }
}

function main() : void {
    let a = new A();
    assert (a.foo() == 1);
    assert (A.foo() == 2);

    let b = new B();
    assert (b.foo() == 3);
    assert (B.foo() == 4);
}
