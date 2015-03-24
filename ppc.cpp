
#include "ppc.h"
#include "m33.h"
#include "tmesh.h"
#include "framebuffer.h"

#include <fstream>
#include <string>
#include <iostream>
#include <sstream>

#define PI 3.14159265f

/* Constructor */
PPC::PPC(float hfov, int _w, int _h) : w(_w), h(_h) {

	C = V3(0.0f, 0.0f, 0.0f);
	a = V3(1.0f, 0.0f, 0.0f);
	b = V3(0.0f, -1.0f, 0.0f);
	float alpha = hfov / 2.0f * PI / 180.0f;
	c = V3(-(float)w / 2.0f, (float)h / 2.0f, -(float)w / (2.0f *tanf(alpha)));

}

/* Check if there's valid projection */
bool PPC::Project(V3 P, V3 &PP) {

	M33 cam;
	cam.SetColumn(0, a);
	cam.SetColumn(1, b);
	cam.SetColumn(2, c);

	V3 q = cam.inverse() * (P - C);

	if (q[2] <= 0.0f)
		return false;

	PP[0] = q[0] / q[2];
	PP[1] = q[1] / q[2];
	PP[2] = 1.0f / q[2];
	return true;

}
void PPC::PositionAndOrient(V3 newC, V3 lap, V3 vpv) {


	V3 newVD = (lap - newC).UnitVector();

	V3 newa = (newVD ^ vpv).UnitVector();
	newa = newa * a.Length();

	V3 newb = (newVD ^ newa).UnitVector();
	newb = newb * b.Length();

	V3 newc = newVD*GetF() - newa*(float)w / 2.0f
		- newb*(float)h / 2.0f;

	a = newa;
	b = newb;
	c = newc;
	C = newC;

}

V3 PPC::GetVD() {

	V3 ret;
	ret = (a ^ b).UnitVector();
	return ret;

}

float PPC::GetF() {

	float ret;
	ret = GetVD()*c;
	return ret;

}

/* Rotate pan */
void PPC::PanRotate(float deg){
	V3 tmp = C - b;
	a = a.RotatePointAboutArbitraryAxis(C, tmp, deg);
	b = b.RotatePointAboutArbitraryAxis(C, tmp, deg);
	c = c.RotatePointAboutArbitraryAxis(C, tmp, deg);
	C = C;
}

/* Rotate tilt */
void PPC::TiltRotate(float deg){
	V3 tmp = C + a;
	a = a.RotatePointAboutArbitraryAxis(C, tmp, deg);
	b = b.RotatePointAboutArbitraryAxis(C, tmp, deg);
	c = c.RotatePointAboutArbitraryAxis(C, tmp, deg);
	C = C;
}

/* Rotate roll */
void PPC::RollRotate(float deg){
	V3 ab = a^b;
	V3 tmp = C + ab;
	a = a.RotatePointAboutArbitraryAxis(C, tmp, deg);
	b = b.RotatePointAboutArbitraryAxis(C, tmp, deg);
	c = c.RotatePointAboutArbitraryAxis(C, tmp, deg);
	C = C;
}

/* Translation left-right */
void PPC::LeftRightTranslate(float step){
	a = a;
	b = b;
	c = c;
	C = C + a.UnitVector()*step;
}

/* Translate up-down */
void PPC::UpDownTranslate(float step){
	a = a;
	b = b;
	c = c;
	C = C - b.UnitVector()*step;
	//cerr << C << endl;
}

/* Translate front-back */
void PPC::FrontBackTranslate(float step){
	V3 ab = a^b;
	a = a;
	b = b;
	c = c;
	C = C + ab.UnitVector()*step;
}

/* Get point on image plane */
V3 PPC::GetPointOnImagePlane(float uf, float vf){
	V3 ret;
	ret = C + c + a*uf + b*vf;
	return ret;
}

/* Get point on Focal plane */
V3 PPC::GetPointOnFocalPlane(float uf, float vf, float f){
	float of = GetF();
	V3 ret;
	ret = C + (c + a*uf + b*vf)*f/of;
	return ret;
}

/* Get Principal point of direction u */
float PPC::GetPPU(){
	float ret;
	ret = ((c * -1) * a.UnitVector()) / a.Length();
	return ret;
}

/* Get Principal point of direction v */
float PPC::GetPPV(){
	float ret;
	ret = ((c*-1)*b.UnitVector()) / b.Length();
	return ret;
}

/* Get hfov */
float PPC::GetHfov(){
	float hfov;
	hfov = 2 * atan(w / 2 * a.Length() / GetF());
	return hfov;
}

/* Zoom in and out */
void PPC::Zoom(float zoom){
	float fp = GetF()*zoom;
	V3 vd = GetVD();
	C = C;
	a = a;
	b = b;
	c = a*(-1 * GetPPU()) - b*GetPPV() + vd*fp;
}

/* Interpolation */
PPC PPC::Interpolate(PPC *ppc1, PPC ppci, float i, float n){
	V3 C0, C1, Ci, a0, a1, ai, vd0, vd1, vdi, bi, ci;
	C0 = C;
	C1 = ppc1->C;
	a0 = a;
	a1 = ppc1->a;
	vd0 = GetVD();
	vd1 = ppc1->GetVD();

	Ci = C0 + (C1 - C0)*(float)i / (float)(n - 1);
	vdi = vd0 + (vd1 - vd0)*(float)i / (float)(n - 1);
	ai = a0 + (a1 - a0)*(float)i / (float)(n - 1);
	bi = (vdi^ai).UnitVector()*b.Length();
	ci = ai*(-1 * GetPPU()) - bi*GetPPV() + vdi*GetF();
	PositionAndOrient(Ci, Ci + vdi, ai^vdi);

	ppci.C = Ci;
	ppci.a = ai;
	ppci.b = bi;
	ppci.c = ci;

	return ppci;
}

/* Save to text file */
void PPC::SaveToTxt(char *fname){
	ofstream ofs(fname, fstream::app | fstream::out);
	if (ofs.is_open()){
		ofs << GetHfov() << " "
			<< w << " "
			<< h << " "
			<< C << " "
			<< a << " "
			<< b << " "
			<< c << endl;
		ofs.close();
	}
}

/* Load from text file */
PPC PPC::LoadFromTxt(char *fname){
	ifstream ifs(fname);
	float hfov1, w1, h1;
	V3 C1, a1, b1, c1;
	if (ifs.is_open()){
		ifs >> hfov1 
			>> w1
			>> h1
			>> C1
			>> a1
			>> b1
			>> c1;
		ifs.close();
	}
	PPC ret(hfov1, w1, h1);
	ret.C = C1;
	ret.a = a1;
	ret.b = b1;
	ret.c = c1;
	return ret;
}

/* Load camera path from text file */
list<PPC> PPC::LoadCameraPathFromTxt(char *fname){
	ifstream ifs(fname);
	// V3 C1, a1, b1, c1;
	list<PPC> cameraPath;
	stringstream ss;

	string line;
	if (ifs.is_open()){
		while (getline(ifs,line)){
			float hfov1, w1, h1;
			float C1x, C1y, C1z;
			float a1x, a1y, a1z;
			float b1x, b1y, b1z;
			float c1x, c1y, c1z;
			
			// cerr << line << endl;
			ss << line;
			ss >> hfov1;
			ss >> w1;
			ss >> h1;
			ss >> C1x >> C1y >> C1z;
			ss >> a1x >> a1y >> a1z;
			ss >> b1x >> b1y >> b1z;
			ss	>> c1x >> c1y >> c1z;
			
			ss << "";
			ss.clear();
	
			PPC ppc1 = PPC(hfov1, w1, h1);
			ppc1.C = V3(C1x, C1y, C1z);
			ppc1.a = V3(a1x, a1y, a1z);
			ppc1.b = V3(b1x, b1y, b1z);
			ppc1.c = V3(c1x, c1y, c1z);
			
			//cerr << ppc1 << endl;
			cameraPath.push_back(ppc1);
			
			// cerr << line;
		}
		ifs.close();
	}

	return cameraPath;
}



/* Visualization */
void PPC::Visualize(PPC *ppc1, FrameBuffer *fb, unsigned int color){
	V3 p1, p2, p3, p4;
	p1 = GetPointOnFocalPlane(0, 0, h);
	p2 = GetPointOnFocalPlane(w, 0, h);
	p3 = GetPointOnFocalPlane(w, h, h);
	p4 = GetPointOnFocalPlane(0, h, h);
	// C1 = GetPointOnFocalPlane(w/2, h/2, h);

	V3 c;
	c.SetFromColor(color);
	fb->Draw3DSegment(C, p1, ppc1, c, c);
	fb->Draw3DSegment(C, p2, ppc1, c, c);
	fb->Draw3DSegment(C, p3, ppc1, c, c);
	fb->Draw3DSegment(C, p4, ppc1, c, c);
	fb->Draw3DSegment(p1, p2, ppc1, c, c);
	fb->Draw3DSegment(p2, p3, ppc1, c, c);
	fb->Draw3DSegment(p3, p4, ppc1, c, c);
	fb->Draw3DSegment(p4, p1, ppc1, c, c);

	
}

ostream& operator<<(ostream& os, PPC ppc){
	os
		<< ppc.GetHfov() << " "
		<< ppc.w << " "
		<< ppc.h << " "
		<< ppc.C << " "
		<< ppc.a << " "
		<< ppc.b << " "
		<< ppc.c << " ";
	return os;
}

void PPC::RenderWireframe(PPC *ppc, FrameBuffer *fb, float f,
	unsigned int color) {

	V3 baseCorners[4];
	baseCorners[0] = GetPointOnFocalPlane(0.0f, 0.0f, f);
	baseCorners[1] = GetPointOnFocalPlane(0.0f, (float)h, f);
	baseCorners[2] = GetPointOnFocalPlane((float)w, (float)h, f);
	baseCorners[3] = GetPointOnFocalPlane((float)w, 0.0f, f);
	V3 c0;
	c0.SetFromColor(color);
	for (int si = 0; si < 4; si++) {
		fb->Draw3DSegment(C, baseCorners[si], ppc, c0, c0);
		fb->Draw3DSegment(baseCorners[si], baseCorners[(si + 1) % 4], ppc, c0, c0);
	}

}

void PPC::SetByInterpolation(PPC* ppc0, PPC* ppc1, float frac) {

	V3 newC = ppc0->C + (ppc1->C - ppc0->C)*frac;
	V3 vd0 = ppc0->a ^ ppc0->b;
	V3 vd1 = ppc1->a ^ ppc1->b;
	V3 newvd = (vd0 + (vd1 - vd0)*frac).UnitVector();
	V3 vpv = (ppc0->b + (ppc1->b - ppc0->b)*frac).UnitVector()*-1.0f;
	PositionAndOrient(newC, newC + newvd, vpv);

}

V3 PPC::UnProject(V3 PP) {

	V3 ret;
	ret = C + (a*PP[0] + b*PP[1] + c) / PP[2];
	return ret;

}

void PPC::SetIntrinsicsHW(float nearz, float farz) {

	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float scf = nearz / GetF();
	float left = -(float)w / 2.0f*scf;
	float right = (float)w / 2.0f*scf;
	float top = (float)h / 2.0f*scf;
	float bottom = -(float)h / 2.0f*scf;
	glFrustum(left, right, bottom, top, nearz, farz);
	glMatrixMode(GL_MODELVIEW);

}

void PPC::SetExtrinsicsHW() {

	V3 lap = C + GetVD();
	V3 up = b * -1.0f;
	glLoadIdentity();
	gluLookAt(C[0], C[1], C[2], lap[0], lap[1], lap[2], up[0], up[1], up[2]);

}