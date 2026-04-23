#ifndef FILE_IO_H
#define FILE_IO_H

#include "network.h"
#include <string>

/**
 * @file file_io.h
 * @brief File parsing module for transport network data.
 * 
 * Parses STOP.txt format containing STOP, LINE, and SEGMENT records.
 * Supports both 7-field and 8-field SEGMENT formats.
 */

/**
 * @brief Load transport network from a data file.
 * @param filepath Path to the data file (e.g., "STOP.txt")
 * @return Populated TransportNetwork object
 * @throws std::runtime_error if file cannot be opened or contains critical errors
 */
TransportNetwork loadFromFile(const std::string& filepath);

#endif // FILE_IO_H
