#include <iostream>
#include <fstream>
#include <string>
#include <mutex>

using namespace std;

class Logger
{
    private:
        ofstream logFile;
        mutex logMutex;
    public:
        Logger(const std::string& filename);
       
        ~Logger();
        

        void log(const string& message);
        
};
