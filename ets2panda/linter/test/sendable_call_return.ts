// collections.Array 的 map 方法导致的编译不报错

@Sendable 
class SendableAnimal<T> {};
@Sendable 
class SendableDog<T, U> {};

@Sendable
class SendableData {};
class NormalData {};

function handle1<T>(p:T): SendableAnimal<T> {
    return new SendableAnimal<T>();
}

handle1(new SendableData()); // OK, The inferred return type is legal
handle1(new NormalData()); // ERROR, The inferred return type is the sendable class with 'non-sendable-type'

function handle2<T>(p:T) {
    return new SendableAnimal<T>;
}
handle2(new SendableData()); // OK
handle2(new NormalData()); // ERROR, The inferred return type is the sendable class with 'non-sendable-type'


function handle3<T>(p:T) : SendableDog<number, T> {
    return new SendableDog<number, T>;
}
handle3(new SendableData()); // OK
handle3(new NormalData()); // ERROR, If there are more than one generic parameter, all must be valid.


function handle4<T>(p:T) : SendableAnimal<number> | SendableDog<number, T> {
    return new SendableAnimal<number>() || new SendableDog<number, T>();
}
handle4(new SendableData()); // OK
handle4(new NormalData()); // ERROR, If it is a union type, all must be valid.