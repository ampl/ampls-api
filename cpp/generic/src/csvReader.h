#ifndef CSVREADER_INCLUDE_
#define CSVREADER_INCLUDE_

#include <istream>
#include <string>
#include <vector>
#include <map>


/**
To read the tuple; not used right now, keeping it here
just in case we decide to change map type
*/
std::vector<std::vector<std::string> > readCSV(std::istream& in);

std::map<std::string, int> createMap(std::istream& in, const char* beginWith);

#endif
