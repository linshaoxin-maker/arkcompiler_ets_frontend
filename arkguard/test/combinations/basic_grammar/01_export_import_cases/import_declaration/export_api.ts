export let var1 = 1;
export let var2 = 2;
export let var3 = 3;

export function addFunc(para1: number, para2: number){
  return para1 + para2;
}
export function reduceFunc(para1: number, para2: number){
  return para1 - para2;
}

export default function maxFunc(num1: number, num2: number) {
  return Math.max(num1, num2);
}
