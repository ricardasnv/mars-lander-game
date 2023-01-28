// Ricardas Navickas 2020
#ifndef ERROR_H
#define ERROR_H

#include <string>
#include <iostream>
#include "config.h"

// Prints warning/error/info message
// Output format: "[WARNING|ERROR|INFO]: <src>: <msg>\n"
void warning(std::string src, std::string msg);
void error(std::string src, std::string msg);
void info(std::string src, std::string msg);

// Prints fatal error message and calls exit(1)
// Output format: "[FATAL] <src>: <msg>\n"
void fatal(std::string src, std::string msg);

// Prints debug message iff SHOW_DEBUG_MESSAGES is defined
// Output format: "[DEBUG] <src>: <msg>\n"
void debug(std::string src, std::string msg);

#endif
