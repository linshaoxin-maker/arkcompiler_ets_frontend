declare let a1:number;
declare var a2:string;
declare const a3:boolean;
export const a4=1;

declare class C1 {
  f1:number;
  f2():boolean;
  f3:()=>{}
}

declare interface I {
  f1:number;
  f2():I;
  f3:()=>{}
}

declare type t = number | string | t[];

declare function f1(a:C1, b:{a:E2}):{a:t}

declare enum E1{a,b,c}
declare const enum E2{e,f,g,c}

declare namespace ns {
  let a1:number;
  var a2:string;
  const a3:boolean;
  export const a4=1;

  class C1 {
    f1:number;
    f2():boolean;
    f3:()=>{};
  }

  interface I{
    f1:number;
    f2():ns.I;
    f3:()=>{}
  }

  type t = number | string | t[];
  function f1(a:ns.C1, b:{a:ns.E2}):{a:ns.t}

  enum E1{a=1,b,c}
  const enum E2{e=2,f,g,c}
}