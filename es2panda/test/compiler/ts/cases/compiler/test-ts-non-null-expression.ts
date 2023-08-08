function parse(page) {
    return page;
}

class Page {
    process(context) {
        return parse(context?.page!);
    }
}

function getPropertyAccess(obj) {
    return obj!.property;
}

function getElementAccess(arr, index) {
   return arr![index];
}

function getCall(func, arg) {
    func!(arg);
}

function getCondition(value) {
    return value ? value! : "defaultValue";
}

function getAsExpr(value) {
    return value! as number;
}

print("non-null-test");
