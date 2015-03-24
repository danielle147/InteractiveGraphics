#pragma once

#include "v3.h"
#include "framebuffer.h"
#include "ppc.h"

class TMesh {
public:
	bool enabled, isWF,isTM;
	V3 *verts, *normals, *cols, *tcs, *ttcs;
	int vertsN;
	unsigned int *tris;
	int trisN;
	AABB *aabb;

	TMesh() : enabled(true),isWF(false),isTM(false),verts(0), vertsN(0), tris(0), trisN(0),
		normals(0), cols(0), aabb(0), tcs(0), ttcs(0) {};
	TMesh(V3 center, V3 dims, unsigned int color);
	TMesh(char *fname);
	void LoadBin(char *fname);
	void RenderPoints(PPC *ppc, FrameBuffer *fb, int psize);
	void RenderWireframe(PPC *ppc, FrameBuffer *fb, unsigned int color);
	void SetAABB();
	void Translate(V3 tv);
	V3 GetCenter();
	void Position(V3 newCenter);
	void ScaleToNewDiagonal(float newDiagonal);
	void Scale(float scf);
	void RotateMesh(V3 aO, V3 aD, float thetad);
	void RenderFilled(PPC *ppc, PPC *lightView, FrameBuffer *fb, unsigned int color, 
		V3 L, float ka,float es, 
		int renderMode, int tilingMode, int lookupMode, FrameBuffer *texture);
	V3 SSIVColor(V3 pv, V3 redABC, V3 greenABC, V3 blueABC);
	V3 SSIN(V3 pv, V3 nxABC, V3 nyABC, V3 nzABC);
	float KS(V3 L, V3 C, V3 P, V3 n, float e);
	void SetFromFB(FrameBuffer *fb, PPC *ppc);
	unsigned int NearestNeighborLookUp(float s, float t, FrameBuffer *texture);
	unsigned int BilinearLookUp(float s, float t, FrameBuffer *texture);
	unsigned int TilingMirror(float s, float t, FrameBuffer *texture);
	unsigned int TilingRepeat(float s, float t, FrameBuffer *texture); 
	void TextureMapping(float u, float v, float s, float t,
		FrameBuffer *fb, FrameBuffer *texture, int lookupMode, int tilingMode);
	V3 GetSMCoordinate(PPC *ppc, PPC* lightView, V3 pv, V3 ABC);
	M33 GetNABC(V3 Vs0, V3 Vs1, V3 Vs2, V3 n0, V3 n1, V3 n2);
	V3 TMesh::GetZABC(V3 v0, V3 v1, V3 v2);
	void RenderHW();
};