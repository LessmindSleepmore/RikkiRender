#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "GeometricTypes.h"

class OBJParser {
private:
	std::vector<vec3f> verts;
	std::vector<vec3f> normals;

	/**
	* x:vertex index
	* y:uv index
	* z:normal index
	*/
	std::vector<std::vector<vec3i>> faces;

public:
	OBJParser(const char* filename);
	~OBJParser();

	int nVerts();
	int nFaces();
	vec3f getVert(int i);
	vec3f getNormals(int i);
	std::vector<vec3i> getFace(int i);
};