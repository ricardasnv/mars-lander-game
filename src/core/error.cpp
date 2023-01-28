// Ricardas Navickas 2020
#include "error.h"
#include <cstdlib>

void warning(std::string src, std::string msg) {
	std::cout << "[WARNING] " << src << ": " << msg << std::endl;
	return;
}

void error(std::string src, std::string msg) {
	std::cout << "[ERROR] " << src << ": " << msg << std::endl;
	return;
}

void info(std::string src, std::string msg) {
	std::cout << "[INFO] " << src << ": " << msg << std::endl;
	return;
}

void fatal(std::string src, std::string msg) {
	std::cout << "[FATAL] " << src << ": " << msg << std::endl;
	exit(1);
	return;
}

void debug(std::string src, std::string msg) {
#ifdef SHOW_DEBUG_MESSAGES
	std::cout << "[DEBUG] " << src << ": " << msg << std::endl;
#endif
	return;
}

