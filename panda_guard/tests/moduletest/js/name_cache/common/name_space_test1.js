export function name_space_test_func_regular_export() {
    return 1;
}

function name_space_test_func_2() {
    return 2;
}

export {name_space_test_func_2 as name_space_test_func_regular_redefine_export}

export function name_space_test_func_indirect_export() {
    return 3;
}

export function name_space_test_func_4() {
    return 4;
}
