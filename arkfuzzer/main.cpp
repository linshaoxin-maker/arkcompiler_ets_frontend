#include <iostream>
#include <unistd.h>
#include <fstream>
#include <string.h>
#include <stdio.h>
#include "cov/cov.h"

#define REPRL_CRFD 100 // Control read file decriptor
#define REPRL_CWFD 101 // Control write file decriptor
#define REPRL_DRFD 102 // Data read file decriptor
#define REPRL_DWFD 103 // Data write file decriptor
std::ofstream eslog("es2abc.log", std::ios::trunc);

bool fuzzilli_reprl = true;
#define TESTCASE_JS_FILE_PATH "/home/fuzzilli/fuzzilli/Targets/ES2ABC/out/"
#define JSTESTCASE_FILE_NAME_LEN 64
#define COV_INIT_BUILTINS_EDGES 128
#define RUN_PYTHON_CMD_LEN 1024
#define EXECUTE_JSTESTCASE_BY_PYTHON "cd /home/openharmony/code/arkcompiler/ets_frontend;\
python3 arkfuzzer/gen_abc.py --file %s --es2021 all \
--ark-frontend-binary=../../out/rk3568/clang_x64/arkcompiler/ets_frontend/es2abc \
--ark-frontend=es2panda --product-name=rk3568 --open-fuzzy-mode 2>&1"

int Execute(char *js_name, int name_len)
{
    size_t script_size;
    unsigned action = 0;

    ssize_t nread = read(REPRL_CRFD, &action, 4);
    if (nread != 4 || action != 'cexe') {
        eslog << "REPRL: Unknown action: %u" << action << " line = " << __LINE__ << std::endl;
        return 1; // fail
    }

    if (read(REPRL_CRFD, &script_size, 8) != 8) {
        eslog << "execute read fail!"
              << " line = " << __LINE__ << std::endl;
        return 1;
    }
    if (script_size == 0) {
        return 0;
    }
    char *buffer = new char[script_size + 1];
    char *ptr = buffer;
    size_t remaining = script_size;
    while (remaining > 0) {
        ssize_t rv = read(REPRL_DRFD, ptr, remaining);
        if (rv == 0)
        {
            eslog << "read finished!!!"
                  << " line = " << __LINE__ << std::endl;
            break;
        }
        remaining -= rv;
        ptr += rv;
    }

    // generate js testcase file
    buffer[script_size] = 0;
    static int index = 0;
    snprintf(js_name, name_len - 1, "%s%d.js", TESTCASE_JS_FILE_PATH, index);
    std::string js_testcase_name = js_name;
    std::ofstream js_testcase(js_testcase_name, std::ios::trunc);
    js_testcase << buffer << std::endl;
    delete[] buffer;

    int res = 0;

    // run received js test case by python
    char cmd[RUN_PYTHON_CMD_LEN] = {0};
    char buff[1024] = {0};

    FILE *pr = nullptr;

    snprintf(cmd, RUN_PYTHON_CMD_LEN - 1, EXECUTE_JSTESTCASE_BY_PYTHON, js_name);   
    if ((pr = popen(cmd, "r")) != nullptr) {
        while (fgets(buff, sizeof(buff) - 1, pr) != nullptr) {
            if (!strstr(buff, "Test command")) {
                res = 1;
            }
            break;
        }
    }
    pclose(pr);

    index++;

    return res;
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
    bool res = true;
    fflush(stdout);
    fflush(stderr);
    if (write(REPRL_CWFD, &status, 4) != 4) {
        eslog << "Deal result write fail!" << __LINE__ << std::endl;
        res = false;
    }
    sanitizer_cov_reset_edgeguards();
    
    return res;
}

int main(int argc, char **argv)
{
    int result = 0;
    char js_name[JSTESTCASE_FILE_NAME_LEN] = {0};

    if (!InitialReprl()) {
        return 1;
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