#include "OBJParser.h"

#define MAX_LINE_LENGTH 256

OBJParser::OBJParser(const char* filename) {
	FILE* file;
	fopen_s(&file, filename, "r");
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }

    char line[MAX_LINE_LENGTH];
	while (fgets(line, MAX_LINE_LENGTH, file)) {
        if (line[0] == 'v') {
            float x, y, z;
            sscanf_s(line, "v %f %f %f", &x, &y, &z);
			verts.push_back(vec3f(x, y, z));
        }
		else if (line[0] == 'f') {
			int v1, v2, v3;
            sscanf_s(line, "f %d/%*d/%*d %d/%*d/%*d %d/%*d/%*d", &v1, &v2, &v3);
			faces.push_back(vec3i(--v1, --v2, --v3));
		}
	}
	std::cout << filename << " read finished. " << "Vertex number: " << nVerts() << " Face number: " << nFaces() << std::endl;
}

OBJParser::~OBJParser()
{
}

int OBJParser::nVerts()
{
	return verts.size();
}

int OBJParser::nFaces()
{
	return faces.size();
}

vec3f OBJParser::getVert(int i)
{
	return verts[i];
}

vec3i OBJParser::getFace(int i)
{
	return faces[i];
}
