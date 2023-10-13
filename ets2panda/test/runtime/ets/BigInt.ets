/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

function test_type(): void {
    let num0 = 0n;
    let num1 = 100_100_100_100_100_100n
    let num2 = -57896044618658097711785492504343953926634992332820282019728792003956564819967n

    assert (num0 instanceof bigint)
    assert (num1 instanceof bigint)
    assert (num2 instanceof bigint)
}

function test_assignment(): void {
    let a = -24059059045444224545405903904190343043049209420234290480n
    let b = a
    assert (a instanceof bigint)
    assert (b instanceof bigint)
    assert (a.toString() == b.toString());

    a = 123n;
    assert (a instanceof bigint)
    assert (a.toString() == "123");
    assert (a == 123n);

    const zero = 0n;
    let c = zero;
    assert (zero instanceof bigint)
    assert (c instanceof bigint)
    assert (zero.toString() == c.toString());
}

function test_compare(): void {
    const a = 24400569094091093912039019089543850580328542852805043n
    const b = 34034240244909504590902901119302940942904944029040950n
    const c = 140044940590459049067274048929058908989042385n
    const d = 44493059209094029409209402940924902n

    // Expecting successfull compilation
    // NOTE(kkonsw): add asserts
    44493059209094029409209402940924902n < 140044940590459049067274048929058908989042385n
    44493059209094029409209402940924902n < a
    a < 34034240244909504590902901119302940942904944029040950n
    a < b

    44493059209094029409209402940924902n <= 140044940590459049067274048929058908989042385n
    44493059209094029409209402940924902n <= a
    a <= 34034240244909504590902901119302940942904944029040950n
    a <= b

    44493059209094029409209402940924902n <= 44493059209094029409209402940924902n
    24400569094091093912039019089543850580328542852805043n <= a
    a <= 24400569094091093912039019089543850580328542852805043n
    a <= a

    40044940590459049067274048929058908989042385n > 44493059209094029409209402940924902n
    34034240244909504590902901119302940942904944029040950n > a
    a > 140044940590459049067274048929058908989042385n
    b > a

    40044940590459049067274048929058908989042385n >= 44493059209094029409209402940924902n
    34034240244909504590902901119302940942904944029040950n >= a
    a >= 140044940590459049067274048929058908989042385n
    b >= a

    44493059209094029409209402940924902n <= 44493059209094029409209402940924902n
    24400569094091093912039019089543850580328542852805043n <= a
    a <= 24400569094091093912039019089543850580328542852805043n
    a <= a
}

function test_literals() : void {
    let num0 = 0n
    assert (num0.toString() == "0")

    let num1 = 127n
    assert (num1.toString() == "127")

    let num2 = 32767n
    assert (num2.toString() == "32767")

    let num3 = 2147483647n
    assert (num3.toString() == "2147483647")

    let num4 = 9223372036854775807n
    assert (num4.toString() == "9223372036854775807")

    let num5 = 170141183460469231731687303715884105727n
    assert (num5.toString() == "170141183460469231731687303715884105727")

    let num6 = 57896044618658097711785492504343953926634992332820282019728792003956564819967n
    assert (num6.toString() == "57896044618658097711785492504343953926634992332820282019728792003956564819967")

    let num1_n = -128n
    assert (num1_n.toString() == "-128")

    let num2_n = -32768n
    assert (num2_n.toString() == "-32768")

    let num3_n = -2147483648n
    assert (num3_n.toString() == "-2147483648")

    let num4_n = -9223372036854775808n
    assert (num4_n.toString() == "-9223372036854775808")

    let num5_n = -170141183460469231731687303715884105728n
    assert (num5_n.toString() == "-170141183460469231731687303715884105728")

    let num6_n = -57896044618658097711785492504343953926634992332820282019728792003956564819968n
    assert (num6_n.toString() == "-57896044618658097711785492504343953926634992332820282019728792003956564819968")

    let num1_sep = 1_991_653_125_841_217_555_434419_9091_123000000_3_3313_5775_3282_29n
    assert (num1_sep.toString() == "1_991_653_125_841_217_555_434419_9091_123000000_3_3313_5775_3282_29")

    let num2_sep = -422_12_3333_9844_3333_3443_34111_43434_1111_11_1_3_3_411909_990081n
    assert (num2_sep.toString() == "-422_12_3333_9844_3333_3443_34111_43434_1111_11_1_3_3_411909_990081")

    let num0_t: bigint = 0n
    assert (num0_t.toString() == "0")

    let num1_t: bigint = 57896044618658097711785492504343953926634992332820282019728792003956564819967n
    assert (num1_t.toString() == "57896044618658097711785492504343953926634992332820282019728792003956564819967")

    let num2_t: bigint = -9223372036854775808n
    assert (num2_t.toString() == "-9223372036854775808")

    let num3_t: bigint = 1_991_653_125_841_217_555_434419_9091_123000000_3_3313_5775_3282_29n
    assert (num3_t.toString() == "1_991_653_125_841_217_555_434419_9091_123000000_3_3313_5775_3282_29")

    let num4_t: bigint = -422_12_3333_9844_3333_3443_34111_43434_1111_11_1_3_3_411909_990081n
    assert (num4_t.toString() == "-422_12_3333_9844_3333_3443_34111_43434_1111_11_1_3_3_411909_990081")

    const num0_c = 0n
    assert (num0_c.toString() == "0")

    const num1_c = 1267650600228229401496703205376n
    assert (num1_c.toString() == "1267650600228229401496703205376")

    const num2_c = -1427247692705959881058285969449495136382746624n
    assert (num2_c.toString() == "-1427247692705959881058285969449495136382746624")

    const num3_c = 4_000_000_000_000_000_000_000_100n
    assert (num3_c.toString() == "4_000_000_000_000_000_000_000_100")

    const num4_c: bigint = -7777777_666666_55555_4444_333_22_1n
    assert (num4_c.toString() == "-7777777_666666_55555_4444_333_22_1")
}

function test_cast(): void {
    const v = 1559053
    const b: byte = 44
    const s: short = -17600
    const i: int = 1150483640
    const l: long = -8223372036854775808

    // NOTE(kkonsw): casts currently do not work
}

function test_bigint_methods(): void {
    const b: byte = 44
    const s: short = -17600
    const i: int = 1150483640
    const l: long = -8223372036854775808

    /* Testing BigInt constructor */
    let n0 = new BigInt(0)
    assert n0 == 0n
    assert(n0.toString() == "0")

    let n1 = new BigInt(654093)
    assert(n1.toString() == "654093")
    assert n1 == 654093n

    let n2 = new BigInt(b)
    assert(n2.toString() == "44")
    assert n2 == 44n

    let n3 = new BigInt(s)
    assert(n3.toString() == "-17600")
    assert n3 == -17600n

    let n4 = new BigInt(i)
    assert(n4.toString() == "1150483640")
    assert n4 == 1150483640n

    let n5 = new BigInt(l)
    assert(n5.toString() == "-8223372036854775808")
    assert n5 == -8223372036854775808n

    let dec = new BigInt("-12392320390239294724747283477947923471101032")
    assert dec == -12392320390239294724747283477947923471101032n

    // NOTE(kkonsw): asIntN not implemented
    /* Testing asIntN() static method */

    // NOTE(kkonsw): asUintN not implemented
    /* Testing asUintN() static method */
}

function test_unary(): void {
    let n = -128n
    assert (n.toString() == "-128")

    let a = 123n;
    assert(a == 123n);
    assert(a.toString() == "123");

    a = -123n;
    assert(a == -123n);
    assert(a.toString() == "-123");

    a = 123n
    assert(a == 123n);
    assert(a.toString() == "123");

    a = -a;
    assert(a == -123n);
    assert(a.toString() == "-123");

    a = -a;
    assert(a == 123n);
    assert(a.toString() == "123");

    let e: bigint;
    assert(a == 123n);
    assert(a.toString() == "123");
    e = -a;

    // Check that original value does not change
    assert(a == 123n);
    assert(a.toString() == "123");
    assert(e == -123n);
    assert(e.toString() == "-123");
    e = a;
    assert(e == 123n);
    assert(e.toString() == "123");

    let b = -123n;
    assert(b == -123n)
    assert(b.toString() == "-123");

    let c = -(123n);
    assert(c == -123n)
    assert(c.toString() == "-123");

    // Double unary minus
    let d = -(-123n);
    assert(d == 123n);
    assert(d.toString() == "123");

    // Triple unary minux
    let f = -(-(-123n));
    assert(f == -123n);
    assert(f.toString() == "-123");

    a = new BigInt(321);
    assert(a.toString() == "321")
    assert(a == 321n)

    b = -a;
    assert(a.toString() == "321")
    assert(a == 321n)
    assert(b == -321n);
    assert(b.toString() == "-321");
}

function test_multiplication(): void {
    const a = 23443495146314363289895841n
    const b = 245000234343499329134n
    const c = -245000234343499329134n

    /* Multiplication testing (*) */
    978667632325344545n * 4534000101n 
    a * b 
    a * c 
    a * 0n
    c * 0n
    
    /* Division testing (/) */
    39735235034886462n / 89221422n 
    a / b
    a / c
    0n / a

    let err = false;
    try {
        a / 0n
    } catch (e) {
        if (e instanceof ArithmeticException) {
            err = true
        }
    }

    /* Remainder of the division (%) */
    493433405047004109n / 111114444n 
    a / b
    0n / a

    err = false;
    try {
        a % 0n
    } catch (e) {
        if (e instanceof ArithmeticException) {
            err = true
        }
    }
}

function test_addition_1(): void {
    const a = 97567789101304567800013210071n
    const b = -533923234343411557221n
    const c = 0n;

    /* Minus testing (-) */
    assert (-a == -97567789101304567800013210071n)
    assert (-b == 533923234343411557221n)
    assert (-c == -0n)
    assert (-(-a) == a)
    assert (-(-b) == b)
    assert (-(-c) == c)

    /* Plus testing (+) */
}

function test_addition_2(): void {
    const a = 18446744073709551616n;
    const b = 36893488147419103232n;
    const c = -10000000000000000000n;
    
    /* Addition testing (+) */
    999_999_999_999_999n + 1n
    a + b
    a + a
    a + c
    a + b + b
}

function test_subtraction(): void {
    const a = 18446744073709551616n;
    const b = 36893488147419103232n;
    const c = -10000000000000000000n;

    // /* Subtraction testing (-) */
    999_999_999_999_999n - 1n
    b - a
    a - b
    b - a - a
}

function test_inc_dec(): void {
    const a = 340282366920938463463374607431768211456n
    const b = -2313455919991034304490n
    
    /* Increment testing */
    /* Decrement testing */
}

function test_shift(): void {
    const a = 245599210405555256299145n
    
    /* Testing left shift (<<) */
    a << 100n
    a << 0n

    /* Testing right shift (>>) */
    a >> 60n
    a >> 0n

    /* Testing unsigned right shift (>>>) */
}

function test_bitwise(): void {
    const a = 123456789123456789123456789123456789123456789123456789n
    let b = 123456790n
    const zero = 0n

    /* Testing bitwise NOT (~) */
    
    /* Testing bitwise AND (&) */
    a & 0n
    a & a

    /* Testing bitwise XOR (^) */
    a ^ 0n 
    a ^ a
    a ^ b

    /* Testing bitwise OR (|) */
    a | 0n 
    a | a 
    a | b
}

function main() : void {
    test_type();
    test_assignment();
    test_compare();
    test_literals();
    test_cast();
    test_bigint_methods();
    test_unary();
    test_multiplication();
    test_addition_1();
    test_addition_2();
    test_subtraction();
    test_inc_dec();
    test_shift();
    test_bitwise();
}
