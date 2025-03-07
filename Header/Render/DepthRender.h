#pragma once
#include "Render.h"

class DepthRender : public Render {

	std::vector<vec4f> view_coords;

public:
	DepthRender(int w, int h);

	void geometryVertexShader(int blockidx, int faceidx) override;

	void clampInTriangle(vec3f _cv) override;

	TGAColor fragmentShader() override;

	bool depthStencilTest(int _x, int _y) override;

	float* getZBuffer();
};