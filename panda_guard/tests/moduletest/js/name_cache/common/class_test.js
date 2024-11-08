export class class1 {
    add() {
        return 1;
    }
}

class class2 {
    add() {
        return 2;
    }
}

export function test_class_in_func() {
    class class3 {
        add() {
            return 3;
        }
    }

    let obj2 = new class2();
    let obj3 = new class3();
    return obj2.add() + obj3.add();
}