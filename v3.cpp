#include "v3.h"
#include "m33.h"

#include<math.h>

#define PI 3.14159265f

V3::V3(float x, float y, float z){
	xyz[0] = x;
	xyz[1] = y;
	xyz[2] = z;
}

ostream& operator<<(ostream& os, V3 v){
	os
		<< v.xyz[0] << " "
		<< v.xyz[1] << " "
		<< v.xyz[2];
	return os;
}

istream& operator>>(istream& is, V3 v){
	is >> v.xyz[0];
	is >> v.xyz[1];
	is >> v.xyz[2];
	return is;
}

float& V3::operator[](int i){
	return xyz[i];
}
/* addition with another vector */
V3 V3::operator+(V3 v1){
	V3 ret;
	ret[0] = xyz[0] + v1[0];
	ret[1] = xyz[1] + v1[1];
	ret[2] = xyz[2] + v1[2];
	return ret;
}
/* subtraction with another vector */
V3 V3::operator-(V3 v1){
	V3 ret;
	ret[0] = xyz[0] - v1[0];
	ret[1] = xyz[1] - v1[1];
	ret[2] = xyz[2] - v1[2];
	return ret;
}
/* dot product */
float V3::operator*(V3 v1){
	return
		xyz[0] * v1[0] +
		xyz[1] * v1[1] +
		xyz[2] * v1[2];
}
/* cross product */
V3 V3::cross(V3 v1){
	V3 ret;
	ret[0] = xyz[1] * v1[2] - xyz[2] * v1[1];
	ret[1] = xyz[2] * v1[0] - xyz[0] * v1[2];
	ret[2] = xyz[0] * v1[1] - xyz[1] * v1[0];
	return ret;
}
/* division with a scalar */
V3 V3::operator/(float scf){
	V3 ret(*this);
	ret[0] /= scf;
	ret[1] /= scf;
	ret[2] /= scf;
	return ret;
}
/* multiplication with a scalar */
V3 V3::operator*(float scf){
	V3 ret(*this);
	ret[0] *= scf;
	ret[1] *= scf;
	ret[2] *= scf;
	return ret;
}
/* length */
float V3::length(V3 v1){
	return
		sqrt(pow(v1[0] - xyz[0], 2) + pow(v1[1] - xyz[1], 2) + pow(v1[2] - xyz[2], 2));
}
/* normalization */
V3 V3::normalize(V3 v1){
	V3 ret;
	float len = this->length(v1);
	ret[0] = (v1[0] - xyz[0]) / len;
	ret[1] = (v1[1] - xyz[1]) / len;
	ret[2] = (v1[2] - xyz[2]) / len;
	return ret;
}
/* Another version of normalization */
V3 V3::Normalized(){
	V3 ret(*this);
	float len = ret.Length();
	ret[0] = ret[0] / len;
	ret[1] = ret[1] / len;
	ret[2] = ret[2] / len;
	return ret;
}

/* change coordinate system */
V3 V3::changeCoord(V3 o, V3 x, V3 y, V3 z){
	M33 m;
	m[0] = V3(x[0], x[1], x[2]);
	m[1] = V3(y[0], y[1], y[2]);
	m[2] = V3(z[0], z[1], z[2]);
	V3 v = V3(this->xyz[0] - o[0], this->xyz[1] - o[1], this->xyz[2] - o[2]);
	V3 p = m*v;
	return p;
}
/* rotate point about arbitrary axis */
V3 V3::rotateP(V3 o, V3 a, float deg){
	V3 x, y, b, c;
	x = V3(1, 0, 0);
	y = V3(0, 1, 0);
	/* choose the axis with smaller cross product with a */
	(x*a < y*a) ? x = x : x = y;
	/* create new coordinate system with origin o, and a as one of its axis */
	a = normalize(a);
	b = normalize(x.cross(a));
	c = normalize(a.cross(b));
	/* transform p to the new coordinate system p-> pt */
	V3 pt = changeCoord(o, a, b, c);
	/* rotate about a(first axis),pt->pp */
	V3 pp = M33(0, deg)*pt;
	/* transform back to original coordinate system pp->pr */
	V3 origin, xo, yo, zo;
	origin = V3(0, 0, 0);
	xo = V3(1, 0, 0);
	yo = V3(0, 1, 0);
	zo = V3(0, 0, 1);
	V3 pr = pp.changeCoord(origin, xo, yo, zo);
	return pr;
}
/* rotate vector about arbitrary axis */
V3 V3::rotateV(V3 o, V3 a, float deg){
	V3 x, y, b, c, origin;
	origin = V3(0, 0, 0);
	x = V3(1, 0, 0);
	y = V3(0, 1, 0);
	(x*a < y*a) ? x = x : x = y;
	a = normalize(a);
	b = normalize(x.cross(a));
	c = normalize(a.cross(b));
	/* transform p to the new coordinate system p-> pt */
	V3 pt = changeCoord(o, a, b, c);
	/* transform origin to the new coordinate system origin->ot */
	V3 ot = origin.changeCoord(o, a, b, c);
	/* rotate about a, pt->pp; ot->oo */
	M33 rotM = M33(0, deg);
	V3 pp, oo;
	pp = rotM*pt;
	oo = rotM*ot;
	/* transform back to original coordinate system pp->pr, oo->oor */
	V3 xo, yo, zo, pr, oor;
	xo = V3(1, 0, 0);
	yo = V3(0, 1, 0);
	zo = V3(0, 0, 1);
	pr = pp.changeCoord(origin, xo, yo, zo);
	oor = oo.changeCoord(origin, xo, yo, zo);
	/* calculate the return vector and return it */
	V3 ret = V3(pr[0] - oor[0], pr[1] - oor[1], pr[2] - oor[2]);
	return ret;
}
/* Set from color*/
void V3::SetFromColor(unsigned int color){
	V3 &v = *this;
	v[0] = ((unsigned char*)&color)[0];
	v[1] = ((unsigned char*)&color)[1];
	v[2] = ((unsigned char*)&color)[2];
	v = v / 255.0f;
}
/* Get color*/
unsigned int V3::GetColor(){
	V3 &v = *this;
	unsigned int red = (int)(v[0] * 255.0f + 0.5f);
	unsigned int green = (int)(v[1] * 255.0f + 0.5f);
	unsigned int blue = (int)(v[2] * 255.0f + 0.5f);

	if (red < 0)
		red = 0;
	else if (red>255)
		red = 255;
	if (green < 0)
		green = 0;
	else if (green>255)
		green = 255;
	if (blue < 0)
		blue = 0;
	else if (blue>255)
		blue = 255;

	unsigned int ret = 0xFF000000 + blue * 256 * 256 + green * 256 + red;
	return ret;
}
unsigned int V3::GetColorGrey(){
	V3 &v = *this;
	unsigned int red = (int)(v[0] * 255.0f + 0.5f);
	unsigned int green = (int)(v[0] * 255.0f + 0.5f);
	unsigned int blue = (int)(v[0] * 255.0f + 0.5f);
	
	if (red < 0)
		red = 0;
	else if (red>255)
		red = 255;
	if (green < 0)
		green = 0;
	else if (green>255)
		green = 255;
	if (blue < 0)
		blue = 0;
	else if (blue>255)
		blue = 255;

	unsigned int ret = 0xFF000000 + blue * 256 * 256 + green * 256 + red;
	return ret;
}

AABB::AABB(V3 firstPoint) {

	corners[0] = corners[1] = firstPoint;

}

void AABB::AddPoint(V3 pt) {

	if (corners[0][0] > pt[0]) {
		corners[0][0] = pt[0];
	}
	if (corners[0][1] > pt[1]) {
		corners[0][1] = pt[1];
	}
	if (corners[0][2] > pt[2]) {
		corners[0][2] = pt[2];
	}


	if (corners[1][0] < pt[0]) {
		corners[1][0] = pt[0];
	}
	if (corners[1][1] < pt[1]) {
		corners[1][1] = pt[1];
	}
	if (corners[1][2] < pt[2]) {
		corners[1][2] = pt[2];
	}

}

V3 V3::operator^(V3 op1) {

	V3 ret;
	ret.xyz[0] = xyz[1] * op1.xyz[2] - xyz[2] * op1.xyz[1];
	ret.xyz[1] = xyz[2] * op1.xyz[0] - xyz[0] * op1.xyz[2];
	ret.xyz[2] = xyz[0] * op1.xyz[1] - xyz[1] * op1.xyz[0];

	return ret;

}

float V3::Length() {

	float ret;
	V3 &v = (*this);
	ret = sqrtf(v*v);
	return ret;

}

V3 V3::UnitVector() {

	V3 ret = (*this) / Length();
	return ret;

}

V3 V3::RotatePointAboutArbitraryAxis(V3 aO, V3 aD,
	float thetad) {

	V3 xaxis(1.0f, 0.0f, 0.0f);
	V3 yaxis(0.0f, 1.0f, 0.0f);

	float adx = fabsf(xaxis*aD);
	float ady = fabsf(yaxis*aD);
	V3 aux;
	if (adx < ady) {
		aux = xaxis;
	}
	else {
		aux = yaxis;
	}

	M33 lm;
	lm[0] = (aux^aD).Normalized();
	lm[1] = aD.Normalized();
	lm[2] = (lm[0] ^ lm[1]).Normalized();

	M33 ilm = lm.Inverse();

	M33 rotY; 
	rotY.SetRotationY(thetad);

	V3 pt(*this);
	V3 lpt = lm*(pt - aO);
	V3 rlpt = rotY*lpt;
	V3 ret = aO + ilm*rlpt;
	return ret;

}


V3 V3::RotateDirectionAboutArbitraryAxis(V3 aD, float thetad) {

	V3 xaxis(1.0f, 0.0f, 0.0f);
	V3 yaxis(0.0f, 1.0f, 0.0f);

	float adx = fabsf(xaxis*aD);
	float ady = fabsf(yaxis*aD);
	V3 aux;
	if (adx < ady) {
		aux = xaxis;
	}
	else {
		aux = yaxis;
	}

	M33 lm;
	lm[0] = (aux^aD).Normalized();
	lm[1] = aD.Normalized();
	lm[2] = (lm[0] ^ lm[1]).Normalized();

	M33 ilm = lm.inverse();

	M33 rotY; rotY.SetRotationY(thetad);

	V3 pt(*this);
	V3 lpt = lm*pt;
	V3 rlpt = rotY*lpt;
	V3 ret = ilm*rlpt;
	return ret;

}

bool AABB::Clip(float lf, float rf, float tf, float bf) {


	if (corners[1][0] < lf)
		return false;
	if (corners[0][0] > rf)
		return false;
	if (corners[1][1] < tf)
		return false;
	if (corners[0][1] > bf)
		return false;

	if (corners[0][0] < lf)
		corners[0][0] = lf;
	if (corners[1][0] > rf)
		corners[1][0] = rf;
	if (corners[0][1] < tf)
		corners[0][1] = tf;
	if (corners[1][1] > bf)
		corners[1][1] = bf;

	return true;

}

bool V3::IntersectPlaneWithRayWithThisOrigin(V3 rayDirection,
	V3 planeNormal, V3 planePoint, V3 &intersectionPoint) {

	V3 &P = *this;
	float t = ((planePoint - P)*planeNormal) / (rayDirection * planeNormal);
	if (t < 0.0f)
		return false;
	intersectionPoint = P + rayDirection * t;
	return true;

}

V3 V3::IntersectRayWithTriangleWithThisOrigin(V3 r, V3 Vs[3]) {

	M33 m;
	m.SetColumn(0, Vs[1] - Vs[0]);
	m.SetColumn(1, Vs[2] - Vs[0]);
	m.SetColumn(2, r*-1.0f);
	V3 ret;
	V3 &C = *this;
	ret = m.Inverse() * (C - Vs[0]);
	return ret;

}