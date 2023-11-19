// @target: es2015
namespace ts {
    let friendA: { getX(o: A): number, setX(o: A, v: number): void };
    
    
    class A { 
      x: number;
    
      constructor (v: number) {
        this.x = v;
      }
    
      getX () {
        return this.x;
      }
    
       obj() {
        friendA = {
          getX(obj) { return obj.x },
          setX(obj, value) { obj.x = value }
        };
      }
    };

    class B {
      constructor(public a: A, private x1: number = 1, protected x2: string = '', readonly x3: number = 2) {
        const x = friendA.getX(a); // ok
    
        friendA.setX(a, x + 1); // ok
      }
    };

    const a = new A(41);
    a.obj();
    const b = new B(a);
    a.getX();
}