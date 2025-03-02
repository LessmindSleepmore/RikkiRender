#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include "GeometricTypes.h"

class OBJParser {
private:
	std::vector<vec3f> verts;
	std::vector<vec3f> normals;
	std::vector<vec2f> uv;
	std::vector<std::string> materials;
	std::vector<unsigned char*> textures;
	std::vector<std::vector<int>> textures_params;
	std::unordered_map<int, int> block2texture;

	/**
	* 层次：第i块部分-第j个面片-三个顶点的顶点索引，UV索引和法线索引。
	* 
	* x:vertex index
	* y:uv index
	* z:normal index
	*/
	std::vector<std::vector<std::vector<vec3i>>> faces;

	void getTexture(const char* filename);

public:
	OBJParser(const char* filename);
	~OBJParser();

	int nVerts();
	int nFaces(int block);
	int nBlock();

	/**
	* Input: blockidx-块索引,uv-归一化uv坐标.
	* Output: vec4<unsigned char>,范围0-255.
	*/
	vec4c samplerTexture2D(int blockidx, vec2f uv);
	int fromBlockIdx2TextureIdx(int blockidx);
	vec3f getVert(int i);
	vec3f getNormals(int i);
	vec2f getUV(int i);
	std::vector<vec3i> getFace(int block, int i);
};