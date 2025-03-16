#pragma once
#include "RenderPipeline.h"

class DepthRender : public RenderPipeline {

	std::vector<vec4f> view_coords;

public:
	DepthRender(int w, int h);

	void geometryVertexShader(int blockidx, int faceidx) override;

	void clampInTriangle(vec3f _cv) override;

	TGAColor fragmentShader(int x, int y) override;

	bool depthStencilTest(int _x, int _y) override;

	float* getZBuffer();
};