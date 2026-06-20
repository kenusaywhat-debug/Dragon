#include "logger.h"

Logger::~Logger()
{
    if (logFile.is_open())
    {
        logFile.close();
    }
}

Logger::Logger(const std::string& filename)
{
    logFile.open(filename, ios::app);
    if (!logFile.is_open())
    {
        cerr << "Error opening log file: " << filename << endl;
    }
}

void Logger::log(const std::string& message)
{
    lock_guard<mutex> lock(logMutex);
    if (logFile.is_open())
    {
        logFile << message << endl;
        logFile.flush();
    }
}