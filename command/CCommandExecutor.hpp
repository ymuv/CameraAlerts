#pragma once
#include <QString>
#include <mutex>

namespace NCommand
{
struct CCommand;

class CCommandExecutor
{
public: //static methods
    static CCommandExecutor& getInstance();
public:
    void doCommand(const CCommand& command, const QString& args);
private:
    CCommandExecutor();

private:
    std::mutex mMutex;
};
}

