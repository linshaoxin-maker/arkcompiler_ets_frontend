/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

function test_bitwise_and(): void {
    assert new BigInt("10").operatorBitwiseAnd(new BigInt("2")).toString().equals("2");
    assert new BigInt("256").operatorBitwiseAnd(new BigInt("1")).toString().equals("0");
    assert new BigInt("3124378143267041203423").operatorBitwiseAnd(new BigInt("43621978")).toString().equals("41948250");
    assert new BigInt("256").operatorBitwiseAnd(new BigInt("256")).toString().equals("256");
    assert new BigInt("12345678").operatorBitwiseAnd(new BigInt("1234")).toString().equals("66");
}

function test_bitwise_or(): void {
    assert new BigInt("10").operatorBitwiseOr(new BigInt("2")).toString().equals("10");
    assert new BigInt("256").operatorBitwiseOr(new BigInt("1")).toString().equals("257");
    assert new BigInt("256").operatorBitwiseOr(new BigInt("256")).toString().equals("256");
    assert new BigInt("3124378143267041203423").operatorBitwiseOr(new BigInt("43621978")).toString().equals("3124378143267042877151");
    assert new BigInt("12345678").operatorBitwiseOr(new BigInt("1234")).toString().equals("12346846");
}

function test_bitwise_xor(): void {
    assert new BigInt("10").operatorBitwiseXor(new BigInt("2")).toString().equals("8");
    assert new BigInt("256").operatorBitwiseXor(new BigInt("1")).toString().equals("257");
    assert new BigInt("256").operatorBitwiseXor(new BigInt("256")).toString().equals("0");
    assert new BigInt("3124378143267041203423").operatorBitwiseXor(new BigInt("43621978")).toString().equals("3124378143267000928901");
    assert new BigInt("12345678").operatorBitwiseXor(new BigInt("1234")).toString().equals("12346780");
}

function test_left_shift(): void {
    assert new BigInt("0").operatorLeftShift(new BigInt("0")).toString().equals("0");
    assert new BigInt("0").operatorLeftShift(new BigInt("1")).toString().equals("0");
    assert new BigInt("1").operatorLeftShift(new BigInt("0")).toString().equals("1");
    assert new BigInt("10").operatorLeftShift(new BigInt("2")).toString().equals("40");
    assert new BigInt("255").operatorLeftShift(new BigInt("41")).toString().equals("560750930165760");
    assert new BigInt("65535").operatorLeftShift(new BigInt("60")).toString().equals("75556710804409716572160");
    assert new BigInt("4294967295").operatorLeftShift(new BigInt("5")).toString().equals("137438953440");
    assert new BigInt("18446744073709551615").operatorLeftShift(new BigInt("6")).toString().equals("1180591620717411303360");
    assert new BigInt("1275418875248948586535904902545412130").operatorLeftShift(new BigInt("123")).toString().equals("13562579802667292602585801202159372574330573695725523059072421474640855040");
    assert new BigInt("2").operatorLeftShift(new BigInt("218")).toString().equals("842498333348457493583344221469363458551160763204392890034487820288");
    assert new BigInt("-1").operatorLeftShift(new BigInt("0")).toString().equals("-1");
    assert new BigInt("-12").operatorLeftShift(new BigInt("4")).toString().equals("-192");
    assert new BigInt("-255").operatorLeftShift(new BigInt("19")).toString().equals("-133693440");
    assert new BigInt("-65535").operatorLeftShift(new BigInt("73")).toString().equals("-618960574909724398159134720");
    assert new BigInt("-4294967295").operatorLeftShift(new BigInt("24")).toString().equals("-72057594021150720");
    assert new BigInt("-18446744073709551615").operatorLeftShift(new BigInt("31")).toString().equals("-39614081257132168794624491520");
    assert new BigInt("-4095059032950818422890113130149234924134").operatorLeftShift(new BigInt("103")).toString().equals("-41528832328721100931613913139905162112381494314462183326283215847555072");
}

function test_right_shift(): void {
    assert new BigInt("-200").operatorRightShift(new BigInt("2")).toString().equals("-50");
    assert new BigInt("-12").operatorRightShift(new BigInt("2")).toString().equals("-3");
    assert new BigInt("-1").operatorRightShift(new BigInt("0")).toString().equals("-1");
    assert new BigInt("0").operatorRightShift(new BigInt("0")).toString().equals("0");
    assert new BigInt("0").operatorRightShift(new BigInt("1")).toString().equals("0");
    assert new BigInt("1").operatorRightShift(new BigInt("0")).toString().equals("1");
    assert new BigInt("55").operatorRightShift(new BigInt("2")).toString().equals("13");
    assert new BigInt("-50").operatorRightShift(new BigInt("2")).toString().equals("-13");
    assert new BigInt("255").operatorRightShift(new BigInt("4")).toString().equals("15");
    assert new BigInt("255").operatorRightShift(new BigInt("8")).toString().equals("0");
    assert new BigInt("65535").operatorRightShift(new BigInt("10")).toString().equals("63");
    assert new BigInt("4294967295").operatorRightShift(new BigInt("17")).toString().equals("32767");
    assert new BigInt("4294967295").operatorRightShift(new BigInt("48")).toString().equals("0");
    assert new BigInt("18446744073709551615").operatorRightShift(new BigInt("55")).toString().equals("511");
    assert new BigInt("4930493049034092989191918392837727383823827").operatorRightShift(new BigInt("129")).toString().equals("7244");
    assert new BigInt("34930693049034092980065918370009389341341932481328231493102392100010239").operatorRightShift(new BigInt("41")).toString().equals("15884640128676479880626138024130973163365493965706369405371");
    assert new BigInt("34095405903900293499034024029409200000000000000000000094049304394284993939382949384984981018480144184891848991934893429489324901148449849382222222222222888384932890000103010381803018300103180381030084545225875678328499213403403984398439489384984287113383827387373382328328194394892948392222219791749717949183748237").operatorRightShift(new BigInt("511")).toString().equals("5085903091997271218878067997324523590835536101241386224503069830700984460490307918626942225457432781938207872710051592009243523341752202627425621983245162030428");
    assert new BigInt("-255").operatorRightShift(new BigInt("2")).toString().equals("-64");
    assert new BigInt("-65535").operatorRightShift(new BigInt("12")).toString().equals("-16");
    assert new BigInt("-4294967295").operatorRightShift(new BigInt("18")).toString().equals("-16384");
    assert new BigInt("-18446744073709551615").operatorRightShift(new BigInt("38")).toString().equals("-67108864");
    assert new BigInt("-4095059032950818422890113130149234924134").operatorRightShift(new BigInt("99")).toString().equals("-6460863952");
    assert new BigInt("-3294302940940294094923040592059302590950294502940294029029409029429042942049028100104029439420990952").operatorRightShift(new BigInt("100")).toString().equals("-2598746800062401791237527125433630339355687972972274247737341685088307");
}

function test_module(): void {
    assert new BigInt("10").operatorModule(new BigInt("3")).toString().equals("1");
    assert new BigInt("10").operatorModule(new BigInt("-3")).toString().equals("1");
    assert new BigInt("-10").operatorModule(new BigInt("3")).toString().equals("-1");
    assert new BigInt("-10").operatorModule(new BigInt("-3")).toString().equals("-1");
    assert new BigInt("100").operatorModule(new BigInt("50")).toString().equals("0");
    assert new BigInt("100").operatorModule(new BigInt("-50")).toString().equals("0");
    assert new BigInt("-100").operatorModule(new BigInt("50")).toString().equals("0");
    assert new BigInt("-100").operatorModule(new BigInt("-50")).toString().equals("0");
    assert new BigInt("3124378143267041203423").operatorModule(new BigInt("43621978")).toString().equals("18802883");
    assert new BigInt("-3124378143267041203423").operatorModule(new BigInt("43621978")).toString().equals("-18802883");
    assert new BigInt("3124378143267041203423").operatorModule(new BigInt("-43621978")).toString().equals("18802883");
    assert new BigInt("-3124378143267041203423").operatorModule(new BigInt("-43621978")).toString().equals("-18802883");
    assert new BigInt("100").operatorModule(new BigInt("250")).toString().equals("100");
    assert new BigInt("-100").operatorModule(new BigInt("250")).toString().equals("-100");
    assert new BigInt("100").operatorModule(new BigInt("-250")).toString().equals("100");
    assert new BigInt("-100").operatorModule(new BigInt("-250")).toString().equals("-100");
    assert new BigInt("0").operatorModule(new BigInt("8")).toString().equals("0");
}

function test_divide(): void {
    assert new BigInt("10").operatorDivide(new BigInt("3")).toString().equals("3");
    assert new BigInt("-10").operatorDivide(new BigInt("3")).toString().equals("-3");
    assert new BigInt("10").operatorDivide(new BigInt("-3")).toString().equals("-3");
    assert new BigInt("-10").operatorDivide(new BigInt("-3")).toString().equals("3");
    assert new BigInt("100").operatorDivide(new BigInt("50")).toString().equals("2");
    assert new BigInt("100").operatorDivide(new BigInt("-50")).toString().equals("-2");
    assert new BigInt("-100").operatorDivide(new BigInt("50")).toString().equals("-2");
    assert new BigInt("-100").operatorDivide(new BigInt("-50")).toString().equals("2");
    assert new BigInt("3124378143267041203423").operatorDivide(new BigInt("43621978")).toString().equals("71623944775430");
    assert new BigInt("-3124378143267041203423").operatorDivide(new BigInt("43621978")).toString().equals("-71623944775430");
    assert new BigInt("3124378143267041203423").operatorDivide(new BigInt("-43621978")).toString().equals("-71623944775430");
    assert new BigInt("-3124378143267041203423").operatorDivide(new BigInt("-43621978")).toString().equals("71623944775430");
    assert new BigInt("100").operatorDivide(new BigInt("250")).toString().equals("0");
    assert new BigInt("100").operatorDivide(new BigInt("-250")).toString().equals("0");
    assert new BigInt("-100").operatorDivide(new BigInt("250")).toString().equals("0");
    assert new BigInt("-100").operatorDivide(new BigInt("-250")).toString().equals("0");
    assert new BigInt("65000").operatorDivide(new BigInt("100")).toString().equals("650");
    assert new BigInt("65000").operatorDivide(new BigInt("-100")).toString().equals("-650");
    assert new BigInt("-65000").operatorDivide(new BigInt("100")).toString().equals("-650");
    assert new BigInt("-65000").operatorDivide(new BigInt("-100")).toString().equals("650");
}

function test_unary_minus(): void {
    assert new BigInt("10").negate().toString().equals("-10");
    assert new BigInt("1000").negate().toString().equals("-1000");
    assert new BigInt("-1").negate().toString().equals("1");
    assert new BigInt("-10").negate().toString().equals("10");
    assert new BigInt("-100").negate().toString().equals("100");
    assert new BigInt("0").negate().toString().equals("0");
}

function test_plus(): void {
    assert new BigInt("10").operatorAdd(new BigInt("20")).toString() == "30";
    assert new BigInt("1000").operatorAdd(new BigInt("10")).toString() == "1010";
    assert new BigInt("-10").operatorAdd(new BigInt("9")).toString() == "-1";
    assert new BigInt("-10").operatorAdd(new BigInt("10")).toString() == "0";
    assert new BigInt("-100").operatorAdd(new BigInt("10")).toString() == "-90";
    assert new BigInt("100").operatorAdd(new BigInt("10")).toString() == "110";
    assert new BigInt("65535").operatorAdd(new BigInt("65535")).toString() == "131070";
    assert new BigInt("65500").operatorAdd(new BigInt("1")).toString().equals("65501");
    assert new BigInt("65500").operatorAdd(new BigInt("-1")).toString().equals("65499");
    assert new BigInt("-65500").operatorAdd(new BigInt("-1")).toString().equals("-65501");
    assert new BigInt("-65500").operatorAdd(new BigInt("1")).toString().equals("-65499");
    assert new BigInt("-65500").operatorAdd(new BigInt("100000")).toString().equals("34500");
    assert new BigInt("100").operatorAdd(new BigInt("0")).toString().equals("100");
    assert new BigInt("-100").operatorAdd(new BigInt("0")).toString().equals("-100");
    assert new BigInt("-10").operatorAdd(new BigInt("-10")).toString().equals("-20");
}

function test_minus(): void {
    assert new BigInt("10").operatorSubtract(new BigInt("2")).toString().equals("8");
    assert new BigInt("2").operatorSubtract(new BigInt("10")).toString().equals("-8");
    assert new BigInt("-10").operatorSubtract(new BigInt("-2")).toString().equals("-8");
    assert new BigInt("-100").operatorSubtract(new BigInt("1")).toString().equals("-101");
    assert new BigInt("-100").operatorSubtract(new BigInt("-1")).toString().equals("-99");
    assert new BigInt("-1000").operatorSubtract(new BigInt("10")).toString().equals("-1010");
    assert new BigInt("1237840127434312471243").operatorSubtract(new BigInt("234112342314526914672342143621463921469")).toString().equals("-234112342314526913434502016187151450226");
    assert new BigInt("-1237840127434312471243").operatorSubtract(new BigInt("234112342314526914672342143621463921469")).toString().equals("-234112342314526915910182271055776392712");
    assert new BigInt("1237840127434312471243").operatorSubtract(new BigInt("-234112342314526914672342143621463921469")).toString().equals("234112342314526915910182271055776392712");
    assert new BigInt("-1237840127434312471243").operatorSubtract(new BigInt("-234112342314526914672342143621463921469")).toString().equals("234112342314526913434502016187151450226");
    assert new BigInt("-1000").operatorSubtract(new BigInt("-10")).toString().equals("-990");
    assert new BigInt("-100").operatorSubtract(new BigInt("0")).toString().equals("-100");
    assert new BigInt("0").operatorSubtract(new BigInt("-100")).toString().equals("100");
    assert new BigInt("0").operatorSubtract(new BigInt("100")).toString().equals("-100");
    assert new BigInt("65500").operatorSubtract(new BigInt("1")).toString().equals("65499");
    assert new BigInt("65500").operatorSubtract(new BigInt("-1")).toString().equals("65501");
    assert new BigInt("-65500").operatorSubtract(new BigInt("-1")).toString().equals("-65499");
    assert new BigInt("-65500").operatorSubtract(new BigInt("1")).toString().equals("-65501");
    assert new BigInt("65500").operatorSubtract(new BigInt("100000")).toString().equals("-34500");
    assert new BigInt("49798775").operatorSubtract(new BigInt("43621978")).toString().equals("6176797");
    assert new BigInt("10").operatorSubtract(new BigInt("20")).toString().equals("-10");
}

function test_multiply(): void {
    assert new BigInt("10").operatorMultiply(new BigInt("10")).toString() == "100";
    assert new BigInt("0").operatorMultiply(new BigInt("50")).toString() == "0";
    assert new BigInt("1").operatorMultiply(new BigInt("50")).toString() == "50";
    assert new BigInt("50").operatorMultiply(new BigInt("5")).toString() == "250";
    assert new BigInt("50").operatorMultiply(new BigInt("-5")).toString() == "-250";
    assert new BigInt("-1").operatorMultiply(new BigInt("-5")).toString() == "5";
    assert new BigInt("0").operatorMultiply(new BigInt("0")).toString() == "0";
    assert new BigInt("123").operatorMultiply(new BigInt("1")).toString() == "123";
    assert new BigInt("1234").operatorMultiply(new BigInt("987")).toString() == "1217958";
    assert new BigInt("3241847031247230147213740214703214721047312").operatorMultiply(new BigInt("412343124123421347812304712431421204731024")).toString() == "1336753332794721625246945391107220242430725631478413717131017736872102322242538207488";
    assert new BigInt("-3241847031247230147213740214703214721047312").operatorMultiply(new BigInt("-412343124123421347812304712431421204731024")).toString() == "1336753332794721625246945391107220242430725631478413717131017736872102322242538207488";
    assert new BigInt("-3241847031247230147213740214703214721047312").operatorMultiply(new BigInt("412343124123421347812304712431421204731024")).toString() == "-1336753332794721625246945391107220242430725631478413717131017736872102322242538207488";
    assert new BigInt("3241847031247230147213740214703214721047312").operatorMultiply(new BigInt("-412343124123421347812304712431421204731024")).toString() == "-1336753332794721625246945391107220242430725631478413717131017736872102322242538207488";
    assert new BigInt("256").operatorMultiply(new BigInt("256")).toString() == "65536";
}

function test_create_empty_bigint(): void {
    let a = new BigInt()
    assert a.positive()
    assert a.toString() == "0"

    let b = new BigInt("")
    assert b.positive()
    assert b.toString() == "0"
}

function test_invalid_bigint(): void {
    // NOTE(kkonsw): implement validation
}

function test_bigint_as_string(): void {
    assert new BigInt("10").toString() == "10"
    assert new BigInt("1000").toString() == "1000"
    assert new BigInt("-1000").toString() == "-1000"
    assert new BigInt("-1").toString() == "-1"
    assert new BigInt("-10").toString() == "-10"
    assert new BigInt("-100").toString() == "-100"
    assert new BigInt("-100000000000000").toString() == "-100000000000000"
    assert new BigInt("0").toString() == "0"
}

function main() : void {
    test_create_empty_bigint();
    test_bigint_as_string();
    test_invalid_bigint();
    test_multiply();
    test_plus();
    test_minus();
    test_unary_minus();
    test_divide();
    test_module();
    test_bitwise_and();
    test_bitwise_or();
    test_bitwise_xor();
    test_left_shift();
    test_right_shift();
}
