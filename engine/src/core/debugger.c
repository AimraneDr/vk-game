#include "core/debugger.h"
#include <stdio.h>
#include <stdarg.h>

#include "core/clock.h"

#ifdef _WIN32
#include <windows.h>
#endif

static const char* logLevelStrings[] = {
    "TRACE",
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "FATAL"
};

#ifdef _WIN32
static WORD logLevelColors[] = {
    8,  // grey
    10, // green
    7, // white
    6, // Yellow
    12, // Red
    79  // red bg
};
#else
static const char* logLevelColors[] = {
    "\x1b[90m", // grey
    "\x1b[32m", // green
    "\x1b[97m", // white
    "\x1b[33m", // Yellow
    "\x1b[31m", // Red
    "\x1b[41m"  // red bg
};
#endif

void logMessage(LogLevel level, const char* message, ...){
    va_list args;
    va_start(args, message);

    debug.totalMessages++;
    debug.counters[level]++;

    Time currentT = clock_get_system_time();

#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
    WORD saved_attributes;

    // Save current attributes
    GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
    saved_attributes = consoleInfo.wAttributes;

    // Set color
    SetConsoleTextAttribute(hConsole, logLevelColors[level]);

    // Print message
    printf(
        "[%u:%u:%u.%u]-[%s] : ", 
        currentT.hours,currentT.minutes,currentT.seconds,currentT.milliseconds, 
        logLevelStrings[level]);
    vprintf(message, args);
    printf("\n");

    // Restore original attributes
    SetConsoleTextAttribute(hConsole, saved_attributes);
#else
    // Print message with color
    printf("%s[%s]-[%s] : ", logLevelColors[level], "00:00:00.00", logLevelStrings[level]);
    vprintf(message, args);
    printf("\x1b[0m\n"); // Reset color
#endif

    va_end(args);
}