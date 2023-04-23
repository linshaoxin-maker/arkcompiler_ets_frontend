/**---
 description: >
    The BindingIdentifier is optional only when the function declaration occurs in an export default declaration
 ---*/
import add from "./function_declarations_1";

Assert.equal(add(0, 1), 1);
Assert.equal(add("0", 1), "01");
Assert.equal(add("0", "1"), "01");
