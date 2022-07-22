### File Introduction

### Environment preparation
1. Compile the ets_frontend warehouse in the root directory of the Hongmeng system code. First delete the out directory, and then execute the ./build.sh --product-name hispark_taurus_standard --build-target ark_js_host_linux_tools_packages --build-target ark_ts2abc_build command to compile. <br>
2. Enter the tool build directory in the out file cd out/hispark_taurus/clang_x64/ark/ark/build, and use the npm install command to build the environment<br>
3. After setting up the environment, go to the Hongmeng system arkcompiler/ets_frontend directory<br>


### Execute the test framework
1. Execute Options<br>
1.1 Perform full testing<br>
python3 ./testJs/run_testJs.py <br>
1.2 Execute the directory test<br>
python3 ./testJs/run_testJs.py --dir file directory, eg (./testJs/test/moduletest)<br>
1.3 Execute a single file test<br>
python3 ./testJs/run_testJs.py --file file path, eg (./testJs/test/moduletest/arr/arr.js)<br>