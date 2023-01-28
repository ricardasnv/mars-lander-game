// Ricardas Navickas 2020
#include "fileio.h"
#include "error.h"
#include "graphics.h"
#include <fstream>

char* read_file(const char* path) {
	std::ifstream f(path);
	unsigned int length;
	char* str;

	if (!f) {
		fatal("read_file()", "Failed to open file " + std::string(path));
	}

	f.seekg(0, std::ios::end);
	length = f.tellg();
	f.seekg(0, std::ios::beg);

	str = new char[length + 1];
	f.read(str, length);
	f.close();

	str[length] = '\0';

	debug("read_file()", "Finished reading file " + std::string(path) + " (" + std::to_string(length) + " bytes)");

	return str;
}

