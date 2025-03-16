#include "../../Header/Data/OBJParser.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define MAX_LINE_LENGTH 256

void OBJParser::getTexture(const char* filename){
    std::string mtl_filename = std::string(filename);
    size_t pos = mtl_filename.find_last_of('.');
    if (pos != std::string::npos) {
        mtl_filename = mtl_filename.substr(0, pos) + ".mtl";
    }
    
    // fopen实在是不方便 :(

    std::ifstream mtlfile(mtl_filename);
    std::unordered_map<std::string, std::string> mt_map;
    std::string cur_mat;
    std::string line;

    while (std::getline(mtlfile, line)) {
        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "newmtl") {
            iss >> cur_mat;
        }
        else if (token == "map_Kd" && !cur_mat.empty()) {
            std::string texfilename;
            iss >> texfilename;

			std::string dirpath;
			size_t pos = mtl_filename.find_last_of('/');
			if (pos != std::string::npos) {
				dirpath = mtl_filename.substr(0, pos + 1);
			}

			mt_map[cur_mat] = dirpath + texfilename;
        }
    }
	
	// 建立块索引和纹理索引的对应关系(一个纹理可能被多个块使用)
	for (int i = 0; i < materials.size(); ++i) {
		auto pos = mt_map.find(materials[i]);
		if (pos != mt_map.end()) {
			block2texture[i] = std::distance(mt_map.begin(), pos);
		}
	}

    mtlfile.close();

    // 读取纹理图片并保存
    for (auto& pair : mt_map) {
        int width, height, channels;
        unsigned char* image = stbi_load(pair.second.c_str(), &width, &height, &channels, 0);
        if (image) {
            std::cout << "Successfully loaded texture " << pair.second << " for material " << pair.first << std::endl;
			textures.push_back(image);
			textures_params.push_back({ width, height, channels });
        }
		else {
			std::cout << "Failed loaded texture " << pair.second << " for material " << pair.first << std::endl;
		}
    }
}

OBJParser::OBJParser(const char* filename) {
	FILE* file;
	fopen_s(&file, filename, "r");
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }

    char line[MAX_LINE_LENGTH];
	std::vector<std::vector<vec3i>> block_faces;

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
		if (line[0] == 'v' && line[1] == 't') {
			float x, y;
			sscanf_s(line, "vt %f %f", &x, &y);
			uv.push_back(vec2f(x, y));
		}
		if (strncmp(line, "usemtl", 6) == 0) {
			char* mat_name = line + 7;
			size_t len = strlen(mat_name);
			if (len > 0 && mat_name[len - 1] == '\n') {
				mat_name[len - 1] = '\0';
			}
			materials.push_back(mat_name);
		}
		if (line[0] == 'f' && line[1] == ' ') {
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

			block_faces.push_back(idx);
		}
		else {
			// 以材质为单位分割面片索引集合
			if (!block_faces.empty()) {
				faces.push_back(block_faces);
				block_faces.clear();
			}
		}
	}

	// 处理最后一部分索引
	if (!block_faces.empty()) {
		faces.push_back(block_faces);
		block_faces.clear();
	}

	std::cout << filename << " read finished. " << "Vertex number: " << nVerts() << " ." << std::endl;
	for (int i = 0; i < materials.size(); ++i) {
		std::cout << "Material " << i << " " << materials[i] << ". Face number: " << nFaces(i) << "." << std::endl;
	}

	// 获取纹理图片
	getTexture(filename);
}

OBJParser::OBJParser()
{
}

OBJParser::~OBJParser()
{
	// 释放stb_image使用的内存
	if (textures.size()) {
		for (auto &t : textures) {
			if (t) {
				stbi_image_free(t);
				t = nullptr;
			}
		}
	}
}

int OBJParser::nVerts()
{
	return verts.size();
}

int OBJParser::nFaces(int block)
{
	return faces[block].size();
}

int OBJParser::nBlock()
{
	return faces.size();
}

vec4c OBJParser::samplerTexture2D(int blockidx, vec2f uv)
{
	uv.x = fmin(fmax(uv.x, 0.), 1.);
	uv.y = fmin(fmax(uv.y, 0.), 1.);

	unsigned char* img = textures[blockidx];

	int x = static_cast<int>(uv.x * (textures_params[blockidx][0] - 1));
	int y = static_cast<int>((1. - uv.y) * (textures_params[blockidx][1] - 1));

	// RGBA
	if (textures_params[blockidx][2] == 4) {
		int index = (y * textures_params[blockidx][0] + x) * textures_params[blockidx][2];
		unsigned char r = img[index];
		unsigned char g = img[index + 1];
		unsigned char b = img[index + 2];
		unsigned char a = img[index + 3];

		return vec4c(r, g, b, a);
	}

	return vec4c();
}

int OBJParser::fromBlockIdx2TextureIdx(int blockidx)
{
	return block2texture[blockidx];
}

vec3f OBJParser::getVert(int i)
{
	return verts[i];
}

vec3f OBJParser::getNormals(int i)
{
	return normals[i];
}

vec2f OBJParser::getUV(int i)
{
	return uv[i];
}

std::vector<vec3i> OBJParser::getFace(int block, int i)
{
	return faces[block][i];
}
