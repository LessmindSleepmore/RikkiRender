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
        if (line[0] == 'v' && line[1] == ' ') {
            float x, y, z;
            sscanf_s(line, "v %f %f %f", &x, &y, &z);
			verts.push_back(vec3f(x, y, z));
        }
		if (line[0] == 'v' && line[1] == 'n') {
			float x, y, z;
			sscanf_s(line, "vn %f %f %f", &x, &y, &z);
			normals.push_back(vec3f(x, y, z));
		}
		else if (line[0] == 'f' && line[1] == ' ') {
			std::vector<vec3i> idx(3);
			sscanf_s(line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
				&idx[0].x, &idx[0].y, &idx[0].z,
				&idx[1].x, &idx[1].y, &idx[1].z,
				&idx[2].x, &idx[2].y, &idx[2].z);

			for (auto& element : idx) {
				element.x -= 1;
				element.y -= 1;
				element.z -= 1;
			}

			faces.push_back(idx);
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

vec3f OBJParser::getNormals(int i)
{
	return normals[i];
}

std::vector<vec3i> OBJParser::getFace(int i)
{
	return faces[i];
}
