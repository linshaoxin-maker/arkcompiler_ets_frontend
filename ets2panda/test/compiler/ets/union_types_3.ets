function main() {
    let x0 : int | String | boolean = 2;
    assert (x0 as int) * 2 == 4: "Error! Must be 4";
    x0 = -2;
    assert (x0 as int) * 3 == -6: "Error! Must be -6";
    x0 = true
    assert (x0 as boolean) == true: "Error! Must be true";
    x0 = "STR1"
    assert ((x0 as String) + "STR2").equals("STR1STR2"): "Error! Must be `STR1STR2`";
    let x1 : String | boolean = "AAA";
    if (x1 instanceof String) {
        assert (x1 as String).equals("AAA"): "Error! Must be `AAA`";
    }
    let x2 : String | boolean | int | double = true;
    assert (x2 as boolean) == true: "Error! Must be `true`";
    let x3 : String | boolean | int | double = 3.14;
    assert (x3 as double) == 3.14: "Error! Must be `3.14`";
}
