#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

class OBJParser {
	OBJParser(const char* filename);
	~OBJParser();

private:
	std::vector vert<Vec3f> verts;
};