#ifndef DP2H_TIMER_H
#include <string>
#include <map>
#include <chrono>
#include <ratio>
#include <iostream>

#define TIME_PRINT_FORMAT(msg, diff) { \
std::cout << std::endl << "#### " << msg << " : " << diff.count() * 1.0 / 1e9 << " seconds ####" << std:: endl; \
std::cout << "#### " << msg << " : " << diff.count() << " nanoseconds ####" << std:: endl << std::endl;                              \
}

#define PRINT_DURATION(msg, content)  { \
auto startTime = std::chrono::high_resolution_clock::now(); \
content;\
auto endTime = std::chrono::high_resolution_clock::now(); \
auto diff = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime); \
TIME_PRINT_FORMAT(msg, diff.count()) \
}

#define GET_DURATION(sum, content)  { \
auto startTime = std::chrono::high_resolution_clock::now(); \
content;\
auto endTime = std::chrono::high_resolution_clock::now(); \
auto diff = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime); \
sum += diff.count(); \
}

namespace cx {

    class Timer {
    public:
        std::map<std::string, std::chrono::high_resolution_clock::time_point> eventMap;
        std::map<std::string, unsigned long long> durationMap;

        void StartTimer(std::string eventName);
        unsigned long long EndTimer(std::string eventName); // milliseconds
        void EndTimerAndPrint(std::string eventName);
        void StopTimerAddDuration(std::string eventName);
        void PrintDuration(std::string eventName);
    };

}

#define DP2H_TIMER_H

#endif //DP2H_TIMER_H
