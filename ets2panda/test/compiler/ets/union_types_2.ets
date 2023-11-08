class A {}

class B {
    num: int = 42;
}

class C {
    num: int = 43;
    constructor(num: int) {
        this.num = num;
    }
    num_to_return() : int {
        return this.num;
    }
}

function foo(x : A | B | C) {
    x.num;  // CTE - `A` doesn't have field `num`
}

function main() {
    let x : A | B | C;
    x = new C(777);
    foo(x);
}
