
var x = {"name": 1, 2: 3}

console.log(x["name"])
console.log(x[2])

class X {
    public name: number = 0
}
let y = {name: 1}
console.log(x.name)

let z = [1, 2, 3]
console.log(y[2])

enum S1 {
  s1 = "qwqwq",
}

enum S2 {
  s2 = 123,
}

interface A1 {
  [S1.s1]: string;
}

interface A2 {
  [S2.s2]: string;
}

const a1: A1 = {
  [S1.s1]: "fld1",
};

const a2: A2 = {
  [S2.s2]: "fld2",
};

S1["s1"];
S2["s2"];
