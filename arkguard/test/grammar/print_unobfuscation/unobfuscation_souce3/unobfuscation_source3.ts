

// -enable-property-obfuscation
// lang

// export
export let export1 = 1;
export let export2 = 2;
export let usb = 3; // lang

// string property
class MyString {
    'string1' = 1;
    'string2' = 2;
    'unit' = 3; // lang
}

// toplevel
function toplevel1(): number {
    return 1;
}
function toplevel2(): number {
    return 2;
}

// property
class MyClass {
    property1 = 1;
    a = 2; // lang
    peoperty2(){}
}

// local variable and parameter
function add(param1: number, param2: number): number { // lang
    let variable1 = param1;
    let variable2 = param2;
    return variable1 + variable2;
}

function myfunc(input: number): number {
    return input + 1;
}
