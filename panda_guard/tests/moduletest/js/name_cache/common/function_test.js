export function function_test1() {
    return 1;
}

function function_test2() {
    return 2;
}

export function test_function_in_func() {
    function function_test3() {
        function function_test3_inner() {
            return 3;
        }

        return function_test3_inner();
    }

    return function_test2() + function_test3();
}