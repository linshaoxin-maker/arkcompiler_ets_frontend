export class propertyClass1 {
    static sField1 = 1;
    field1 = 2;
}

class propertyClass2 {
    static sField2 = 3;
    field2 = 4;
}

export function test_property_in_func() {
    class propertyClass3 {
        static sField3 = 5;
        field3 = 6;
    }

    let obj2 = new propertyClass2();
    return obj2.field2 + propertyClass3.sField3;
}