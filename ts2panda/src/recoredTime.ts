export class RecordTime  {
    static workFileName: string;

    // costTime = createProgramCostTime + programEmitCostTime;
    static startTime: number;
    static endTime: number;
    static costTime: number;

    // cost time by calling create program of tsc
    static createProgramStartTime: number;
    static createProgramEndTime: number;
    static createProgramCostTime: number;

    // cost time by calling program emit of tsc
    // programEmitCostTime = tsc + beforeTotalTime + afterTotalTime
    static programEmitStartTime: number;
    static programEmitEndTime: number;
    static programEmitCostTime: number;

    // cost time by calling program emit before parse of tsc, checkSyntax of ts2panda
    static beforeStartTime: number;
    static beforeEndTime: number;
    static beforeTotalCostTime: number = 0;

    // cost time by calling program emit after parse of tsc
    // afterTotalCostTime =  prepareCompileTotalCostTime + compileAbcTotalCostTime
    static afterStartTime: number;
    static afterEndTime: number;
    static afterTotalCostTime: number = 0;

    static prepareCompileStartTime: number;
    static prepareCompileEndTime: number;
    static prepareCompileTotalCostTime: number = 0;

    // cost time by calling gen abc
    // compileAbcTotalCostTime = recordTotalCostTime + compileTs2abcTotalCostTime
    static compileAbcStartTime: number;
    static compileAbcEndTime: number;
    static compileAbcTotalCostTime: number = 0;

    // record identifer of ast
    static recordStartTime: number;
    static recordEndTime: number;
    static recordTotalCostTime: number = 0;

    // gen bytecode of abc
    static compileCompliationUnitStartTime: number;
    static compileCompliationUnitEndTime: number;
    static compileCompliationTotalCostTime: number = 0;

    // ts2abc process to gen abc file
    static compileTs2abcStartTime: number;
    static compileTs2abcEndTime: number;
    static compileTs2abcTotalCostTime: number = 0;

    // compile json file to abc file
    static compileJsonStartTime: number;
    static compileJsonEndTime: number;
    static compileJsonTotalCostTime: number = 0;
}