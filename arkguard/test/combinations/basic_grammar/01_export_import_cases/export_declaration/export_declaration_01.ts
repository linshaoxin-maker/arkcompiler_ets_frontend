export function foo1() { return 1 }
export function* foo2() {
  yield 1;
  return 2;
}
export async function foo3() { return 3 }
export async function* foo4() { return 4 }

export class className1 {
  prop: string = "hello";
}

export let var1 = 1;
export let var2 = 2, var3 = 3;

export const var4 = 4;
export const var5 = 5, var6 = 6;
