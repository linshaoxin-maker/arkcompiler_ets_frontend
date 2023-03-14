# testJs

### 文件简介

### 环境准备
1. 编译：./build.sh --product-name hispark_taurus_standard --build-target ark_js_host_linux_tools_packages --build-target ark_ts2abc_build命令进行编译。<br>
2. Npm install： cd out/hispark_taurus/clang_x64/ark/ark/build && npm install && cd -<br>


### 执行测试框架
1. 执行选项<br>
1.1 执行全量测试<br>
cd arkcompiler/ets_frontend && python3 ./testJs/run_testJs.py  <br>
1.2 执行目录测试<br>
cd arkcompiler/ets_frontend && python3 ./testJs/run_testJs.py --dir  文件目录，例如（./testJs/test/moduletest）<br>
1.3 执行单个文件测试<br>
cd arkcompiler/ets_frontend && python3 ./testJs/run_testJs.py --file  文件路径，例如（./testJs/test/moduletest/arr/arr.js）<br>
