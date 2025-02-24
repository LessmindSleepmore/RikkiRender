#include "OBJParser.h"

#define MAX_LINE_LENGTH 256

OBJParser::OBJParser(const char* filename) {
	FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }

    char line[MAX_LINE_LENGTH];
	while (fgets(line, MAX_LINE_LENGTH, file)) {
        if (line[0] == 'v') {

        }
	}
}

OBJParser::~OBJParser()
{
}
