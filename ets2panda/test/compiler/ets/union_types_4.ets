class A {
    num: int = 42;
    foo() {}
}

class B {}

function main() {
    let x : String | short | A = 5;
    assert (x as int) * 2 == 10: "Error! Must be 10";
    x = "STRSTR";
    assert (x as String).equals("STRSTR"): "Error! Must be `STRSTR`";
    x = new A();
    assert (x as A).num == 42: "Error! Field of A must be `42`";
    x = new B();  // CTE
}
