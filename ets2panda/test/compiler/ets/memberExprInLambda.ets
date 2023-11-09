type BenchmarkFunc = () => void;

function arrayParamInLambdaFunc(): void {
    const arr: number[] = [1, 2, 3, 4];
    const ITERATE_FUNC: BenchmarkFunc = () => {
        const length = arr.length;
    }
}