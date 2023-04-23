export default function (arg1: string | number, arg2: string | number) {
    if (typeof arg1 === "number" && typeof arg2 === "number") {
        return arg1 + arg2;
    }

    if (typeof arg1 === "string" || typeof arg2 === "string") {
        return `${arg1}${arg2}`;
    }
}
