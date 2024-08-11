// 整体导出
module.exports = {
  exportApi1: 'commonjs',
  exportApi2: (para) => { return para +1}
}

module.exports.api = function() {return 'api'}
module.exports.constVal = 2;

class classExport3 {
  class3_prop1 = 3;
}
module.exports.classExport3Alias = classExport3;
module.exports.exportObj3 = {obj_prop3: 3}

