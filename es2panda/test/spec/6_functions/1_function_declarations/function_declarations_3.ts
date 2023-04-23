export function add(a: string, b: string): string;
export function add(a: number, b: number): number;
export function add(a: string, b: number): string;
export function add(arg1: string | number, arg2: string | number) {
    if (typeof arg1 === "number" && typeof arg2 === "number") {
        return arg1 + arg2;
    }

    if (typeof arg1 === "string" || typeof arg2 === "string") {
        return `${arg1}${arg2}`;
    }
}

