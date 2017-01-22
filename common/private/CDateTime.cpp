#include <QDateTime>
#include <locale>

#include "common/CDateTime.hpp"

std::string CDateTime::getDateTimeAsString()
{
    static const char* fmt = "%Y-%m-%d_%H-%M-%S";
    std::locale::global(std::locale());

    std::time_t t = std::time(NULL);
    char buffer[30];
    std::strftime(buffer, sizeof(buffer), fmt, std::localtime(&t));
    return std::string(buffer);
}

int CDateTime::getCurrentHour()
{
    return QTime::currentTime().hour();
}

int CDateTime::getCurrentMinute()
{
    return QTime::currentTime().minute();
}

int CDateTime::getCurrentSecond()
{
    return QTime::currentTime().second();
}

int CDateTime::getCurrentDayOfWeek()
{
    return QDate::currentDate().dayOfWeek();
}

std::chrono::system_clock::rep CDateTime::getSecondsSinceEpoch()
{
    static_assert(std::is_integral<std::chrono::system_clock::rep>::value,
                  "Representation of ticks isn't an integral value.");
    auto now = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::seconds>(now).count();
}
