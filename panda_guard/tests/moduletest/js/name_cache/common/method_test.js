export class methodClass1 {
    methodAdd1() {
        return 1;
    }
}

class methodClass2 {
    methodAdd2() {
        return 2;
    }
}

class methodClass4 {
    methodAdd4_1() {
        return 1;
    }

    static methodAdd4_2() {
        return 2;
    }

    get methodAdd4_3() {
        return 3;
    }
}

export function test_method_in_func() {
    class methodClass3 {
        methodAdd3() {
            return 3;
        }
    }

    let obj2 = new methodClass2();
    let obj3 = new methodClass3();
    let obj4 = new methodClass4();
    return obj2.methodAdd2() + obj3.methodAdd3() + obj4.methodAdd4_1() + methodClass4.methodAdd4_2() + obj4.methodAdd4_3;
}