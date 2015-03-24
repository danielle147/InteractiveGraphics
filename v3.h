#pragma once

using namespace std;

#include<ostream>
#include<istream>

class V3{
	float xyz[3];
public:
	V3(){};
	V3(float x, float y, float z);
	friend ostream& operator<< (ostream& os, V3 v);
	friend istream& operator>> (istream& is, V3 v);
	float& operator[](int i);
	V3 operator+ (V3 v1);
	V3 operator- (V3 v1);
	float operator* (V3 v1);
	V3 operator/ (float scf);
	V3 operator* (float scf);
	V3 cross(V3 v1);
	float length(V3 v1);
	V3 normalize(V3 v1);
	V3 Normalized();
	V3 changeCoord(V3 o, V3 x, V3 y, V3 z);
	V3 rotateP(V3 o, V3 a, float deg);
	V3 rotateV(V3 o, V3 a, float deg);
	void SetFromColor(unsigned int color);
	unsigned int GetColor();
	unsigned int GetColorGrey();
	float Length();
	V3 operator^(V3 op1);
	V3 UnitVector();
	V3 RotatePointAboutArbitraryAxis(V3 aO, V3 aD, float thetad);
	V3 RotateDirectionAboutArbitraryAxis(V3 aD, float thetad);
	bool IntersectPlaneWithRayWithThisOrigin(V3 rayDirection, V3 planeNormal, V3 planePoint, V3 &intersectionPoint);
	V3 IntersectRayWithTriangleWithThisOrigin(V3 r, V3 Vs[3]);
};

class AABB {
public:
	V3 corners[2];
	AABB(V3 firstPoint);
	void AddPoint(V3 pt);
	bool Clip(float lf, float rf, float tf, float bf);
};
