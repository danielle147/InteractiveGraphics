#pragma once

using namespace std;

#include "v3.h"
#include <istream>
#include <ostream>
#include <list>

class FrameBuffer;

class PPC {
public:
	V3 a, b, c, C;
	int w, h;
	PPC(float hfov, int w, int h);
	bool Project(V3 P, V3 &PP);
	void PositionAndOrient(V3 newC, V3 lap, V3 vpv);
	V3 GetVD();
	float GetF();
	void PanRotate(float deg);
	void TiltRotate(float deg);
	void RollRotate(float deg);
	void LeftRightTranslate(float step);
	void UpDownTranslate(float step);
	void FrontBackTranslate(float step);
	V3 GetPointOnImagePlane(float uf, float vf);
	V3 GetPointOnFocalPlane(float uf, float vf, float f);
	float GetPPU();
	float GetPPV();
	float GetHfov();
	void Zoom(float zoom);
	PPC Interpolate(PPC *ppc1, PPC ppci, float i, float n);
	void Visualize(PPC *ppc1, FrameBuffer *fb, unsigned int color);
	PPC LoadFromTxt(char *fname);
	list<PPC> LoadCameraPathFromTxt(char *fname);
	void SaveToTxt(char*fname);
	friend ostream& operator<< (ostream& os, PPC ppc);
	void RenderWireframe(PPC *ppc, FrameBuffer *fb, float f, unsigned int color);
	void SetByInterpolation(PPC* ppc0, PPC* ppc1, float frac);
	V3 UnProject(V3 PP);
	void SetIntrinsicsHW(float nearz, float farz);
	void SetExtrinsicsHW();
};