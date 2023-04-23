/**---
 description: >
    if the member declaration specifies no value, the member is considered a constant enum member. 
    If the member is the first member in the enum declaration, it is assigned the value zero. 
    Otherwise, it is assigned the value of the immediately preceding member plus one, and an error occurs if the immediately preceding member is not a constant enum member.
    if the member declaration specifies a value that can be classified as a constant enum expression, 
    the member is considered a constant enum member.otherwise, the member is considered a computed enum member.
 ---*/
enum ABCList {
  A,
  B,
  //computed enum member
  C = "string".length,
  D = 10,
  E,
  F = ~17,
  G = 0x0f << 0x02,
  H = 0xff & 0xaa,
  I = E | F,
}
//first member is assigned the value zero
Assert.equal(ABCList.A, 0);
//other member is assigned the value of the immediately preceding member plus one
Assert.equal(ABCList.B, 1);
Assert.equal(ABCList.C, 6);
Assert.equal(ABCList.D, 10);
Assert.equal(ABCList.E, 11);
Assert.equal(ABCList.F, -18);
Assert.equal(ABCList.G, 60);
Assert.equal(ABCList.H, 170);
Assert.equal(ABCList.I, -17);
