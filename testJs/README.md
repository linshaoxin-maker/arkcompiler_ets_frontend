### File Introduction

### Environment preparation
1. Compilation: ./build.sh --product-name hispark_taurus_standard --build-target ark_js_host_linux_tools_packages --build-target ets_frontend_build
2. Npm install: cd out/hispark_taurus/clang_x64/ark/ark/build && npm install && cd -


### Execute the test framework
1. Execute Options<br>
1.1 Perform full testing<br>
cd arkcompiler/ets_frontend && python3 ./testJs/run_testJs.py <br>
1.2 Execute the directory test<br>
cd arkcompiler/ets_frontend && python3 ./testJs/run_testJs.py --dir file directory, eg (./testJs/test/moduletest)<br>
1.3 Execute a single file test<br>
cd arkcompiler/ets_frontend && python3 ./testJs/run_testJs.py --file file path, eg (./testJs/test/moduletest/arrBuffer/arr.js)<br>