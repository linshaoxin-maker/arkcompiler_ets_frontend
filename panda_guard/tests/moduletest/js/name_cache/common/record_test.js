export const recordObject1 = {
    name1: 'a',
    age1: 1
}

const recordObject2 = {
    name2: 'b',
    age2: 2
}

export function test_record_in_func() {
    const recordObject3 = {
        name3: 'c',
        age3: 3
    }

    return recordObject2['age2'] + recordObject3.age3;
}