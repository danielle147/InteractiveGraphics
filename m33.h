#pragma once

using namespace std;

#include <ostream>
#include <istream>

#include "v3.h"

class M33{
	V3 rows[3];
public:
	M33();
	M33(int axis, float deg);
	//M33 rotateA(V3 u, float deg);
	V3& operator[](int i);
	friend ostream& operator<<(ostream& os, M33 m);
	friend istream& operator>>(istream& is, M33 m);
	V3 operator*(V3 v1);
	M33 operator*(M33 m1);
	void Normalize();
	V3 column(int i);
	M33 transpose();
	float det();
	M33 inverse();
	M33 inverseSafe();
	void SetColumn(int i, V3 ci);
	void SetRotationY(float thetad);
	M33 Inverse();
	V3 GetColumn(int i);
};
