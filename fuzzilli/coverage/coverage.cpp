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

#include "coverage.h"

#include <fcntl.h>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define BYTE_NUMBER  8
#define SHM_SIZE 0x100000
#define MAX_EDGES ((SHM_SIZE - 4) * BYTE_NUMBER)

struct ShareMemData {
    uint32_t numEdges;
    unsigned char edges[1];
};

struct ShareMemData* shmem;

uint32_t *edgesStart, *edgesStop;

void SanitizerCovResetEdgeGuards()
{
    uint32_t n = 0;
    for (uint32_t* x = edgesStart; x < edgesStop && n < MAX_EDGES; x++)
        *x = ++n;
}

// SanitizerCoverage fixed interface name
extern "C" void __sanitizer_cov_trace_pc_guard_init(uint32_t* start, uint32_t* stop)
{
    if (shmem) {
        if (!(edgesStart == start && edgesStop == stop)) {
            fprintf(stderr,
                "[COV] Multiple initialization of shmem!"
                " This is probably not intended! Currently only one edge"
                " region is supported\n");
            return;
        }
        // Already initialized.
        return;
    }
    // Map the shared memory region
    const char* shmKey = getenv("SHM_ID");
    if (!shmKey) {
        puts("[COV] no shared memory bitmap available, skipping");
        shmem = reinterpret_cast<struct ShareMemData *>(malloc(SHM_SIZE));
    } else {
        int fd = shm_open(shmKey, O_RDWR, S_IREAD | S_IWRITE);
        if (fd <= -1) {
            fprintf(stderr, "[COV] Failed to open shared memory region\n");
            return;
        }

        shmem = reinterpret_cast<struct ShareMemData *>(mmap(nullptr, SHM_SIZE, PROT_READ |
                                                             PROT_WRITE, MAP_SHARED, fd, 0));
        if (shmem == MAP_FAILED) {
            fprintf(stderr, "[COV] Failed to mmap shared memory region\n");
            return;
        }
    }

    edgesStart = start;
    edgesStop = stop;
    SanitizerCovResetEdgeGuards();

    shmem->numEdges = static_cast<uint32_t>(stop - start);
    printf("[COV] edge counters initialized. Shared memory: %s with %u edges\n", shmKey, shmem->numEdges);
}

uint32_t SanitizerCovCountDiscoveredEdges()
{
    uint32_t onEdgesCounter = 0;
    for (uint32_t i = 1; i < 1 + shmem->numEdges; ++i) {
        const uint32_t byteIndex = i >> 3;  // Divide by 8 using a shift operation
        const uint32_t bitIndex = i & 7;  // Modulo 8 using a bitwise AND operation

        if (shmem->edges[byteIndex] & (1 << bitIndex)) {
            ++onEdgesCounter;
        }
    }
    return onEdgesCounter;
}

// SanitizerCoverage fixed interface name
extern "C" void __sanitizer_cov_trace_pc_guard(uint32_t* guard)
{
    uint32_t index = *guard;
    shmem->edges[index / BYTE_NUMBER] |= 1 << (index % BYTE_NUMBER);
    *guard = 0;
}
