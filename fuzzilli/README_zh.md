# fuzzilli

## 项目说明
 本项目主要目的为fuzzilli适配测试es2abc。
 本项目依赖python3、git、swift、ets_runtime组件下的ark_js_vm可执行文件以及ets_frontend组件下的es2abc可执行文件。
 本项目所有输出在fuzzilli/output目录下。
 
## 用法说明
 执行python脚本run_fuzzy.py，自动执行fuzzilli源码下载、swift工具下载、补丁更新、编译操作以及运行fuzzilli。
 
## 运行示例
$:cd arkcompiler/ets_frontend/
$:python fuzzilli/run_fuzzy.py

## 注意事项
 本项目依赖ets_runtime组件下的ark_js_vm可执行文件以及ets_frontend组件下的es2abc可执行文件