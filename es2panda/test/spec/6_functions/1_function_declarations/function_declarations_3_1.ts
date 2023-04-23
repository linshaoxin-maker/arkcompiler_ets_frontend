/**---
description: >
    All declarations for the same function must specify the same set of modifiers (the same combination of declare, export, and default)
 ---*/
import { add } from "./function_declarations_3";
// (number,number)
Assert.equal(add(0, 1), 1);
// (string,number)
Assert.equal(add("0", 1), "01");
// (string,string)
Assert.equal(add("0", "1"), "01");
