
@Sendable
class Sc1 {

}

@Sendable 
function sf1() {

};

@Sendable
export class Sc2 {

}

@Sendable 
export function sf2() {

};

@Sendable
class Sc3 {

}

@Sendable
function sf3() {

};

@Sendable
class Sc4 {

}

@Sendable
function sf4() {

};

@Sendable
class Sc5 {

}

@Sendable
function sf5() {

};


@Sendable 
class SendableClass {
    handle() {
        new Sc1(); // OK
        sf1(); // OK
        new Sc2(); // WARING, export decl
        sf2(); // WARING, export decl
        new Sc3(); // WARING, export { decl }
        sf3(); // WARING, export { decl }
        new Sc4(); // WARING, export { decl as alias }
        sf4(); // WARING, export { decl as alias }
        new Sc5(); // WARING, export default decl
    }
}

export {Sc3, sf3, Sc4 as Sc42, sf4 as sf42};


export default Sc5;

export * from './module';