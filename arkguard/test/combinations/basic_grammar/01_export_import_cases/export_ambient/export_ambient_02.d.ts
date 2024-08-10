// 这不再是环境模块声明了，而是模块增强！
import {express} from 'path1'
export {};
declare module "path1" {
  export function normalize2(p: string): string;
  export function join2(...paths: any[]): string;
  export var sep: string;
  interface express {
    prop2: number;
  }
}

class ExpressClass implements express {
  prop1: number;
  prop2: number;
  constructor() {}
}