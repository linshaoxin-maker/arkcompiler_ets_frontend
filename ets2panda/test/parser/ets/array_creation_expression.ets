class Point {
    x: number;
    y: number;
}

class ListTest<T> {
    constructor()
    {
        this.num = 10;
        let data = new T[this.num];
    }
    private data: T[];
    private num: int;
}

function main(): void {
    let s = new string[3];
    let n = new number[3];
    let i = new Int[3];
    let x = new Point[3];
    let y = new ListTest<Point>[3];
}


