#include <string>
#include <cstdarg>  // For va_list, va_start, va_end
#include "CrySystem\Logger.h" // Logger

class ServerLog {
public:
    static void Log(const std::string& prefix, const char* format, va_list args) {
        std::string fullFormat = prefix + format;
        Logger::GetInstance().LogV(ILog::eAlways, fullFormat.c_str(), args);
    }
};