/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "os/file.h"
#include "timers.h"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace panda::es2panda::util {
TimeStartFunc Timer::timerStart;
TimeEndFunc Timer::timerEnd;
std::unordered_map<std::string_view, TimeRecord> Timer::timers_;
std::vector<std::string_view> Timer::events_;
std::mutex Timer::mutex_;
std::string Timer::perfFile_;

void Timer::InitializeTimer(std::string &perfFile)
{
    if (!perfFile.empty()) {
        Timer::timerStart = Timer::TimerStartImpl;
        Timer::timerEnd = Timer::TimerEndImpl;
        perfFile_ = perfFile;
    } else {
        Timer::timerStart = Timer::TimerStartDoNothing;
        Timer::timerEnd = Timer::TimerEndDoNothing;
    }
}

void WriteFile(std::stringstream &ss, std::string &perfFile)
{
    std::ofstream fs;
    fs.open(panda::os::file::File::GetExtendedFilePath(perfFile));
    if (!fs.is_open()) {
        std::cerr << "Failed to open perf file: " << perfFile << ". Errro: " << std::strerror(errno) << std::endl;
        return;
    }
    fs << ss.str();
    fs.close();
}

bool DescentComparator(std::pair<std::string, double> p1, std::pair<std::string, double> p2)
{
    return p1.second > p2.second;
}

void Timer::PrintTimers()
{
    std::stringstream ss;
    ss << "------------- Compilation time consumption in milliseconds: -------------" << std::endl;
    ss << "Note: When compiling multiple files in parallel, " <<
          "we will track the time consumption of each file individually. The output will aggregate these times, " <<
          "potentially resulting in a total time consumption greater than the sum of individual file times." <<
          std::endl << std::endl;

    std::vector<std::string> summedUpTimeString;
    ss << "------------- Compilation time consumption of each file: ----------------" << std::endl;

    for (auto &event: events_) {
        auto &timeRecord = timers_.at(event);
        auto formattedEvent =
            std::string(timeRecord.level, ' ') + std::string(timeRecord.level, '#') + std::string(event);

        double eventTime = 0.0;
        std::vector<std::pair<std::string, double>> eachFileTime;

        // ss << "****event: " << event << std::endl;
        for (auto &[file, timePointRecord] : timeRecord.timePoints) {
            auto t = std::chrono::duration_cast<std::chrono::milliseconds>(
                timePointRecord.endTime - timePointRecord.startTime).count();

            // ss << "****file: " << file << " ,start: " <<
            //     std::chrono::duration_cast<std::chrono::milliseconds>(timePointRecord.startTime.time_since_epoch()).count() <<
            //     " ,end: " << std::chrono::duration_cast<std::chrono::milliseconds>(timePointRecord.endTime.time_since_epoch()).count() << std::endl;
            eventTime += t;
            if (!file.empty()) {
                eachFileTime.push_back(std::pair(file, t));
            }
        }

        // print each file time consumption in descending order
        if (!eachFileTime.empty()) {
            std::sort(eachFileTime.begin(), eachFileTime.end(), DescentComparator);
            ss << formattedEvent << ", time consumption of each file:" << std::endl;
        }
        for (auto &pair : eachFileTime) {
            if (!pair.first.empty()) {
                ss << pair.first << ": " << pair.second << " ms" <<std::endl;
            }
        }

        // collect the sum of each file's time consumption
        std::stringstream eventSummedUpTimeStream;
        eventSummedUpTimeStream << formattedEvent << ": " << eventTime << " ms";
        summedUpTimeString.push_back(eventSummedUpTimeStream.str());
    }
    ss << "------------- Compilation time consumption summed up: -------------------" << std::endl;
    for (auto &str: summedUpTimeString) {
        ss << str << std::endl;
    }
    ss << "-------------------------------------------------------------------------" << std::endl;
    WriteFile(ss, perfFile_);
}

void Timer::TimerStartImpl(const std::string_view event, std::string fileName)
{
    TimePointRecord tpr;
    tpr.startTime = std::chrono::steady_clock::now();
    int level = 0;
    try {
        level = eventMap.at(event);
    } catch (std::exception &error) {
        std::cerr << "Undefiend event: " << event << ". Please check!" << std::endl;
    }

    std::unique_lock<std::mutex> lock(mutex_);
    auto iter = timers_.find(event);
    if (iter != timers_.end()) {
        iter->second.timePoints.emplace(fileName, tpr);
    } else {
        TimeRecord tr;
        tr.timePoints.emplace(fileName, tpr);
        tr.event = event;
        tr.level = level;

        timers_.emplace(event, tr);
        events_.push_back(event);
    }
}

void Timer::TimerEndImpl(const std::string_view event, std::string fileName)
{
    auto endTime = std::chrono::steady_clock::now();

    std::unique_lock<std::mutex> lock(mutex_);
    try {
        auto &timeRecord = timers_.at(event);
        auto &timePoint = timeRecord.timePoints.at(fileName);
        timePoint.endTime = endTime;
    } catch (std::exception &error) {
        std::cerr << "Event " << event << " and file " << fileName <<
            " start timer not found in records, skip record end time!" << std::endl;
    }
}

}  // namespace panda::es2panda::util
