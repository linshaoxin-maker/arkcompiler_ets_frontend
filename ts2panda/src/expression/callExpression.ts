/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import * as ts from "typescript";
import { CacheList, getVregisterCache } from "../base/vregisterCache";
import { Compiler } from "../compiler";
import { VReg } from "../irnodes";
import { compileSuperCall, compileSuperProperty } from "../statement/classStatement";
import { createArrayFromElements } from "./arrayLiteralExpression";
import { getObjAndProp } from "./memberAccessExpression";
import { isMemberExpression } from "../base/util";


export function compileCallExpression(expr: ts.CallExpression, compiler: Compiler, inTailPos?: boolean) {
    let pandaGen = compiler.getPandaGen();
    // import call should be supported further

    if ((expr.expression.kind == ts.SyntaxKind.CallExpression) || (expr.expression.kind == ts.SyntaxKind.NewExpression)) {
        let processed = compiler.compileFunctionReturnThis(<ts.NewExpression | ts.CallExpression>expr.expression);
        if (processed) {
            return;
        }
    }

    if (expr.expression.kind == ts.SyntaxKind.SuperKeyword) {
        let args: VReg[] = [];
        let hasSpread = emitCallArguments(compiler, expr, args);
        compileSuperCall(compiler, expr, args, hasSpread);
        pandaGen.freeTemps(...args);
        return;
    }

    let { arguments: args, passThis: passThis } = getHiddenParameters(expr.expression, compiler);

    // compile arguments of function call
    emitCall(expr, args, passThis, compiler);
    pandaGen.freeTemps(...args);
}

export function getHiddenParameters(expr: ts.Expression, compiler: Compiler) {
    let pandaGen = compiler.getPandaGen();
    let passThis = false;
    let args: VReg[] = [];
    let funcReg = pandaGen.getTemp();
    if (isMemberExpression(expr)) {
        passThis = true;
        let thisReg = pandaGen.getTemp();
        let propReg = pandaGen.getTemp();
        let { obj: obj, prop: prop } = getObjAndProp(<ts.PropertyAccessExpression | ts.ElementAccessExpression>expr, thisReg, propReg, compiler);

        if ((<ts.PropertyAccessExpression | ts.ElementAccessExpression>expr).expression.kind == ts.SyntaxKind.SuperKeyword) {
            compileSuperProperty(compiler, expr, thisReg, prop);
        } else {
            pandaGen.loadObjProperty(expr, thisReg, prop);
        }
        pandaGen.storeAccumulator(expr, funcReg);
        args.push(...[funcReg, thisReg]);

        pandaGen.freeTemps(propReg);
    } else {
        compiler.compileExpression(expr);
        pandaGen.storeAccumulator(expr, funcReg);
        args.push(funcReg);
    }
    return { arguments: args, passThis: passThis };
}

function emitCallArguments(compiler: Compiler, expr: ts.CallExpression, args: VReg[]) {
    let pandaGen = compiler.getPandaGen();
    let hasSpread = false;
    for (let i = 0; i < expr.arguments.length; i++) {
        let argument = expr.arguments[i];
        hasSpread = ts.isSpreadElement(argument) ? true : false;
        if (hasSpread) {
            break;
        }
    }

    if (!hasSpread) {
        expr.arguments.forEach((argExpr: ts.Expression) => {
            let arg = pandaGen.getTemp();
            compiler.compileExpression(argExpr);
            pandaGen.storeAccumulator(argExpr, arg);
            args.push(arg);
        });
    }

    return hasSpread;
}

export function emitCall(expr: ts.CallExpression, args: VReg[], passThis: boolean, compiler: Compiler) {
    let pandaGen = compiler.getPandaGen();
    let hasSpread = emitCallArguments(compiler, expr, args);

    if (!hasSpread) {
        pandaGen.call(expr, [...args], passThis);
        return;
    }

    // spread argument exist
    let callee = args[0];
    let thisReg = passThis ? args[1] : getVregisterCache(pandaGen, CacheList.undefined);
    let argArray = pandaGen.getTemp();
    createArrayFromElements(expr, compiler, <ts.NodeArray<ts.Expression>>expr.arguments, argArray);
    pandaGen.callSpread(expr, callee, thisReg, argArray);
    pandaGen.freeTemps(argArray);
}