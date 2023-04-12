//
// Created by ChenXin on 2021/3/24.
//

#include "timer.h"

void cx::Timer::StartTimer(std::string eventName) {
    eventMap[eventName] = std::chrono::high_resolution_clock::now();
}

unsigned long long cx::Timer::EndTimer(std::string eventName) {
    auto endTime = std::chrono::high_resolution_clock::now();

    if (eventMap.find(eventName) == eventMap.end()) {
        printf("event not exists\n");
        exit(88);
    }

    auto diff = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - eventMap[eventName]);
    return diff.count();
}

void cx::Timer::EndTimerAndPrint(std::string eventName) {
    auto endTime = std::chrono::high_resolution_clock::now();

    if (eventMap.find(eventName) == eventMap.end()) {
        printf("event not exists\n");
        exit(88);
    }

    auto diff = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - eventMap[eventName]);
    std::cout << std::endl << "#### " << eventName << " : " << diff.count() * 1.0 / 1e9 << " seconds ####" << std:: endl;
    std::cout << "#### " << eventName << " : " << diff.count() << " nanoseconds ####" << std:: endl << std::endl;
}

void cx::Timer::StopTimerAddDuration(std::string eventName) {
    auto diff = EndTimer(eventName);
    if (durationMap.find(eventName) == durationMap.end()) {
        durationMap[eventName] = diff;
    } else {
        durationMap[eventName] += diff;
    }
}

void cx::Timer::PrintDuration(std::string eventName) {
    std::cout << std::endl << "#### " << eventName << " : " << durationMap[eventName] * 1.0 / 1e9 << " seconds ####" << std:: endl;
    std::cout << "#### " << eventName << " : " << durationMap[eventName] << " nanoseconds ####" << std:: endl << std::endl;
}
