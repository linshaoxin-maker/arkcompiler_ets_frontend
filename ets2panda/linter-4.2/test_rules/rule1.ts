
var x = { 'name': 1, 2: 3 };

console.log(x['name']);
console.log(x[2]);

class X {
  public name: number = 0;
}
let y = { name: 1 };
console.log(x.name);

let z = [1, 2, 3];
console.log(y[2]);

enum S1 {
  SS = 'qwqwq'
}

enum S2 {
  SS = 123
}

interface A1 {
  [S1.SS]: string;
}

interface A2 {
  [S2.SS]: string;
}

const a1: A1 = {
  [S1.SS]: 'fld1'
};

const a2: A2 = {
  [S2.SS]: 'fld2'
};

S1['SS'];
S2['SS'];
