// 整体导出
module.exports = {
  exportApi1: 'commonjs',
  exportApi2: (para) => { return para +1}
}
// 指定导出
class classExport1 {
  class1_prop1: number;
}
exports.classExport1 = classExport1;
exports.arrowFunc = (para: number) => { 3.14 * para * 2};

module.exports.api = function() {}
module.exports.constVal = 2;

// 导出类型中有属性
module.exports = class {
  class2_prop1 = 1;
}
class classExport3 {
  class3_prop1: number;
}
module.exports.classExport3Alias = classExport3;
module.exports = {obj_prop1: 1, obj_prop2: {inner_prop1: 2}}
module.exports.exportObj3 = {obj_prop3: 3}
