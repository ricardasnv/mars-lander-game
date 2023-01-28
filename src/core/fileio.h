// Ricardas Navickas 2020
#ifndef FILEIO_H
#define FILEIO_H

// Loads contents of file on the heap
// Returns pointer to file data
char* read_file(const char* path);

// Load texture from file
// Returns GL object id of new texture
unsigned int load_texture(const char* imgpath);

#endif
