/**
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "coverage/coverage.h"
#include "securec.h"

#include <iostream>
#include <unistd.h>
#include <fstream>
#include <cstring>
#include <cstdio>

#define REPRL_CRFD 100 // Control read file decriptor
#define REPRL_CWFD 101 // Control write file decriptor
#define REPRL_DRFD 102 // Data read file decriptor
#define REPRL_DWFD 103 // Data write file decriptor
#define REPRL_HELO_LEN  4
#define REPRL_INFO_LEN  8
std::ofstream eslog("es2abc.log", std::ios::trunc);

bool fuzzilliReprl = true;
#define TESTCASE_JS_FILE_PATH "output/"
#define JSTESTCASE_FILE_NAME_LEN 64
#define COV_INIT_BUILTINS_EDGES 128
#define RUN_PYTHON_CMD_LEN 1024
#define EXECUTE_JSTESTCASE_BY_PYTHON "cd ../../;\
python3 fuzzilli/gen_abc.py --file ./fuzzilli/fuzzilli/%s --es2021 all \
--ark-frontend-binary=../../out/rk3568/clang_x64/arkcompiler/ets_frontend/es2abc \
--ark-frontend=es2panda --product-name=rk3568 --open-fuzzy-mode 2>&1"

int Receve(char *jsName, int nameLen, int bufflen)
{
    char *buffer = new char[bufflen + 1];
    char *ptr = buffer;
    size_t remain = bufflen;
    while (remain > 0) {
        ssize_t rv = read(REPRL_DRFD, ptr, remain);
        if (rv == 0) {
            break;
        }
        remain -= rv;
        ptr += rv;
    }

    // generate js testcase file
    buffer[bufflen] = 0;
    static int index = 0;
    int num = 0;
    num = snprintf_s(jsName, nameLen, nameLen - 1, "%s%d.js", TESTCASE_JS_FILE_PATH, index);
    if (num < 0) {
        delete[] buffer;
        return 1;
    }
    std::string jsTestCaseName = jsName;
    std::ofstream jsTestCase(jsTestCaseName, std::ios::trunc);
    jsTestCase << buffer << std::endl;
    delete[] buffer;
    index++;
    return 0;
}

int Execute()
{
    size_t scriptSize;
    unsigned action = 0;
    int res = 0;
    char jsName[JSTESTCASE_FILE_NAME_LEN] = {0};
    ssize_t nRead = read(REPRL_CRFD, &action, REPRL_HELO_LEN);
    if (nRead != REPRL_HELO_LEN || action != 'cexe') {
        eslog << "REPRL: Unknown action: %u" << action << " line = " << __LINE__ << std::endl;
        return 1; // fail
    }
    if (read(REPRL_CRFD, &scriptSize, REPRL_INFO_LEN) != REPRL_INFO_LEN) {
        return 1;
    }
    if (scriptSize == 0) {
        return 0;
    }

    res = Receve(jsName, JSTESTCASE_FILE_NAME_LEN, scriptSize);
    if (res) {
        return 1;
    }

    // run received js test case by python
    char cmd[RUN_PYTHON_CMD_LEN] = {0};
    char buff[1024] = {0};
    FILE *pr = nullptr;
    int num = 0;
    num = snprintf_s(cmd, RUN_PYTHON_CMD_LEN, RUN_PYTHON_CMD_LEN - 1, EXECUTE_JSTESTCASE_BY_PYTHON, jsName);
    if (num < 0) {
        pclose(pr);
        return 1;
    }
    if ((pr = popen(cmd, "r")) != nullptr) {
        if (fgets(buff, sizeof(buff) - 1, pr) != nullptr) {
            if (!strstr(buff, "Test command")) {
                res = 1;
            }
        }
    }
    pclose(pr);

    return res;
}

bool InitialReprl()
{
    char helo[] = "HELO";
    if (write(REPRL_CWFD, helo, REPRL_HELO_LEN) != REPRL_HELO_LEN ||
        read(REPRL_CRFD, helo, REPRL_HELO_LEN) != REPRL_HELO_LEN) {
        eslog << "write or read fail" << __LINE__ << std::endl;
        return false;
    }

    if (memcmp(helo, "HELO", REPRL_HELO_LEN) != 0) {
        eslog << "REPRL: Invalid response from parent" << __LINE__ << std::endl;
        return false;
    }

    return true;
}

bool DealResult(int result)
{
    int status = result << REPRL_INFO_LEN;
    bool res = true;
    fflush(stdout);
    fflush(stderr);
    if (write(REPRL_CWFD, &status, REPRL_HELO_LEN) != REPRL_HELO_LEN) {
        eslog << "Deal result write fail!" << __LINE__ << std::endl;
        res = false;
    }
    SanitizerCovResetEdgeGuards();

    return res;
}

int main(int argc, char **argv)
{
    int result = 0;

    if (!InitialReprl()) {
        return 1;
    }

    do {
        result = Execute();
        if (!DealResult(result)) {
            fuzzilliReprl = false;
            eslog << "DealResult" << __LINE__ << std::endl;
        }
    } while (fuzzilliReprl);

    return 0;
}