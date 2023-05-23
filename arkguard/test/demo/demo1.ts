// comments
interface Square {
    kind: 'square';
    size: number;
}
interface Rectangle {
    kind: 'rectangle';
    width: number;
    height: number;
}
interface Circle {
    kind: 'circle';
    radius: number;
}
console.log(222);
namespace test2 {
    interface Circle2 {
        kind: 'circle';
        radius: number;
    }
    let a: Circle2;
}

