import { SendableAnimal, NormalAnimal} from './sendable_tools_export';


@Sendable 
class SendableClass<T> {
    prop1: Readonly<SendableAnimal> = new SendableAnimal(); // OK
    prop2: Readonly<number> = 1; // ERROR, Generic parameters for tools only support sendable-class or sendable-interface
    prop4: Readonly<NormalAnimal> = new NormalAnimal(); // ERROR, Generic parameters for tools only support sendable-class or sendable-interface
    prop5: Readonly<T>; // ERROR, generics are not supported
    
    constructor(p:T) {
        this.prop5 = p;
    }
}

const a1: Readonly<SendableAnimal> = new SendableAnimal(); // OK
const a2: Readonly<SendableAnimal> = a1; // OK, sendable-tools object can assign to sendable-tools
const a3: Readonly<SendableAnimal> = new NormalAnimal(); // ERROR, The assignment object must be sendable-class or sendable-interface or sendable-tools object
const a4: Readonly<SendableAnimal> = {}; // ERROR
const a5: Readonly<SendableAnimal> = []; // ERROR
const b1: Readonly<NormalAnimal> = new SendableAnimal(); // OK, 
const b2: Readonly<NormalAnimal> = {}; // OK
const c1: Readonly<SendableAnimal> | NormalAnimal = new NormalAnimal(); // ERRPR, The union type only needs to contain one sendable to trigger a check.
const c2: Readonly<SendableAnimal> | number = 5; // OK, The scope of check is 'non-sendable class or interface or tools or {} or [] ';



@Sendable 
class SendableMain {
    prop1: Required<SendableAnimal>;
    prop2: Partial<SendableAnimal>;
    constructor(p1: Required<SendableAnimal>, p2: Partial<SendableAnimal>) {
        this.prop1 = p1;
        this.prop2 = p2;
    }
}

const requiredAnimal: Required<SendableAnimal> = new SendableAnimal();
const partialAnimal: Partial<SendableAnimal> = new SendableAnimal();

new SendableMain(new SendableAnimal(), new SendableAnimal());
new SendableMain(requiredAnimal, partialAnimal); 

