#ifndef DEBUGGER_H
#define DEBUGGER_H
#include "engine/data_types.h"

typedef enum{
    LOG_TRACE,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL
} LogLevel;

typedef struct Debugger_t{
    char** journal;

    u32 counters[6];
    u32 totalMessages;
}Debugger;

static Debugger debug = {0};

void logMessage(LogLevel level, const char* message, ...);

#define LOG_TRACE(...) logMessage(LOG_TRACE, __VA_ARGS__)
#define LOG_DEBUG(...) logMessage(LOG_DEBUG, __VA_ARGS__)
#define LOG_INFO(...) logMessage(LOG_INFO, __VA_ARGS__)
#define LOG_WARN(...) logMessage(LOG_WARN, __VA_ARGS__)
#define LOG_ERROR(...) logMessage(LOG_ERROR, __VA_ARGS__)
#define LOG_FATAL(...) logMessage(LOG_FATAL, __VA_ARGS__)

#endif //DEBUGGER_H