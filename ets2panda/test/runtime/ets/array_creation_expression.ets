class Point {
    x: number;
    y: number;
}

class ListTest<T> {
    constructor()
    {
        this.num = 10;
        let data = new T[this.num]
    }
    private data: T;
    public num: int;
}

function main(): void {
    let stringArr = new String[3];
    let A = stringArr[0] as String;
    assert stringArr[0] == "";
    
    let numberArr = new number[3];
    let B = numberArr[0] as number;
    assert numberArr[0] == 0.0

    let PointArr = new Point[3];
    let C = PointArr[0] as Point;
    assert PointArr[0].x == 0;
    assert PointArr[0].y == 0;

    let ListTestArr = new ListTest<Point>[3];
    let D = ListTestArr[0] as ListTest<Point>;
    assert ListTestArr[0].num == 10;
}
