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
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <string.h>
#include <stdio.h>
#include "cov.h"

#define REPRL_CRFD 100 // Control read file decriptor
#define REPRL_CWFD 101 // Control write file decriptor
#define REPRL_DRFD 102 // Data read file decriptor
#define REPRL_DWFD 103 // Data write file decriptor
std::ofstream eslog("es2abc.log", std::ios::trunc);

bool fuzzilli_reprl = true;
#define TESTCASE_JS_FILE_PATH "./Targets/ES2ABC/out/"
#define JSTESTCASE_FILE_NAME_LEN  32
#define COV_INIT_BUILTINS_EDGES   1024

int Execute(char *js_name, int name_len)
{

    unsigned action = 0;
    ssize_t nread = read(REPRL_CRFD, &action, 4);
    if (nread != 4 || action != 'cexe') {
        eslog << "REPRL: Unknown action: %u" << action << " line = " << __LINE__ << std::endl;
        return 1; // fail
    }
    size_t script_size;
    if (read(REPRL_CRFD, &script_size, 8) != 8) {
        eslog << "execute read fail!"
              << " line = " << __LINE__ << std::endl;
        return 1;
    }
    eslog << "Execute script_size = " << script_size << " line = " << __LINE__ << std::endl;
    if (script_size == 0) {
        return 0;
    }
    char *buffer = new char[script_size + 1];
    char *ptr = buffer;
    size_t remaining = script_size;
    while (remaining > 0) {
        ssize_t rv = read(REPRL_DRFD, ptr, remaining);
        if (rv == 0) {
            eslog << "read finished!!!"
                  << " line = " << __LINE__ << std::endl;
            break;
        }
        remaining -= rv;
        ptr += rv;
    }
    buffer[script_size] = 0;
    // generate js testcase file
    static int index = 0;
    snprintf(js_name, name_len - 1, "%s%d.js", TESTCASE_JS_FILE_PATH, index++);
    std::string js_testcase_name = js_name;
    std::ofstream js_testcase(js_testcase_name, std::ios::trunc);
    js_testcase << buffer << std::endl;

    delete[] buffer;

    return 1;
}

bool InitialReprl()
{
    char helo[] = "HELO";
    if (write(REPRL_CWFD, helo, 4) != 4 || read(REPRL_CRFD, helo, 4) != 4) {
        eslog << "write or read fail" << __LINE__ << std::endl;
        return false;
    }

    if (memcmp(helo, "HELO", 4) != 0) {
        eslog << "REPRL: Invalid response from parent" << __LINE__ << std::endl;
        return false;
    }

    return true;
}

bool DealResult(int result)
{
    int status = result << 8;
    fflush(stdout);
    fflush(stderr);
    if (write(REPRL_CWFD, &status, 4) != 4) {
        eslog << "Deal result write fail!" << __LINE__ << std::endl;
        return false;
    }

    return true;
}

int main(int argc, char **argv)
{
    uint32_t start = 0;
    uint32_t stop = 1024;
    int result = 0;
    char js_name[JSTESTCASE_FILE_NAME_LEN] = {0};

    SanitizerCoverTracePcGuardInit(&start, &stop);
    CoverInitBuiltinsEdges(COV_INIT_BUILTINS_EDGES);
    if (!InitialReprl()) {
        fuzzilli_reprl = false;
    }

    do {
        result = Execute(js_name, sizeof(js_name));

        if (!DealResult(result)) {
            fuzzilli_reprl = false;
            eslog << "DealResult" << __LINE__ << std::endl;
        }
    } while (fuzzilli_reprl);

    return 0;
}