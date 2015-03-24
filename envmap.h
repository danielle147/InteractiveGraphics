#pragma once

#include "framebuffer.h"
#include "ppc.h"
#include "v3.h"


class EnvMap
{
public:
	FrameBuffer *face[6];
	PPC *ppc[6];
	V3 vd[6];

	EnvMap();
	unsigned int GetColor(V3 direction, V3 eye);

};

