#pragma once

#include <chrono>
#include <string>
#include <QString>

class CDateTime
{
public:
    CDateTime() = delete;
    static std::string getDateTimeAsString();

    /**
     * @brief getCurrentHour return current housr [0..23]
     * @return value from 0 to 23
     */
    static int getCurrentHour();

    static int getCurrentMinute();

    static int getCurrentSecond();

    //1 - monday, 7 - sanday, 0 - invalid
    static int getCurrentDayOfWeek();

    static std::chrono::system_clock::rep getSecondsSinceEpoch();
};
