#include "m33.h"
#include "v3.h"

#include <iostream>
#include <stdlib.h>
#include <math.h>

#define PI 3.14159265f

M33::M33(){
	rows[0] = V3(1, 0, 0);
	rows[1] = V3(0, 1, 0);
	rows[2] = V3(0, 0, 1);
}
/* rotate about x,y,z axis with x=0,y=1,z=2 */
M33::M33(int axis, float deg){
	float rad = deg*PI / 180;
	if (axis == 0){
		rows[0] = V3(1, 0, 0);
		rows[1] = V3(0, cos(rad), -sin(rad));
		rows[2] = V3(0, sin(rad), cos(rad));
	}
	else if (axis == 1){
		rows[0] = V3(cos(rad), 0, sin(rad));
		rows[1] = V3(0, 1, 0);
		rows[2] = V3(-sin(rad), 0, cos(rad));
	}
	else if (axis == 2){
		rows[0] = V3(cos(rad), -sin(rad), 0);
		rows[1] = V3(sin(rad), cos(rad), 0);
		rows[2] = V3(0, 0, 1);
	}
	else
		cerr << "axis undefined! Please input another axis value!";
}

V3& M33::operator[](int i){
	return rows[i];
}

ostream& operator<<(ostream& os, M33 m){
	os << m[0] << endl;
	os << m[1] << endl;
	os << m[2] << endl;
	return os;
}

istream& operator>>(istream& is, M33 m){
	is >> m[0] >> m[1] >> m[2];
	return is;
}
/* return column vector given the column number(0-2) */
V3 M33::column(int i){
	V3 ret;
	ret[0] = rows[0][i];
	ret[1] = rows[1][i];
	ret[2] = rows[2][i];
	return ret;
}
/* matrix times column vector */
V3 M33::operator*(V3 v){
	V3 ret(rows[0] * v, rows[1] * v, rows[2] * v);
	return ret;
}
/* matrix multiplication */
M33 M33::operator*(M33 m1){
	M33 ret;
	ret[0] = V3(rows[0] * m1.column(0), rows[0] * m1.column(1), rows[0] * m1.column(2));
	ret[1] = V3(rows[1] * m1.column(0), rows[1] * m1.column(1), rows[1] * m1.column(2));
	ret[2] = V3(rows[2] * m1.column(0), rows[2] * m1.column(1), rows[2] * m1.column(2));
	return ret;
}
/* matrix transposition */
M33 M33::transpose(){
	M33 ret;
	ret[0] = this->column(0);
	ret[1] = this->column(1);
	ret[2] = this->column(2);
	return ret;
}
/* matrix determinant */
float M33::det(){
	return
		this->column(0)*(this->column(1).cross(this->column(2)));
}
/* matrix inversion */
M33 M33::inverse(){
	M33 ret;
	float det = this->det();
	ret[0] = (this->column(1).cross(this->column(2))) / det;
	ret[1] = (this->column(2).cross(this->column(0))) / det;
	ret[2] = (this->column(0).cross(this->column(1))) / det;
	//ret = ret.transpose();
	return ret;

}
/* safe version of matrix inversion */
M33 M33::inverseSafe(){
	if (fabs(this->det()) < 0.00001f){
		cerr << "The matrix is not invertible!" << endl;
		exit(0);
	}
	else
		return this->inverse();
}
/* Normalize matrix */
void M33::Normalize(){
	V3 vs = rows[0] + rows[1] + rows[2];
	float sum = vs[0] + vs[1] + vs[2];

	rows[0] = rows[0] / sum;
	rows[1] = rows[1] / sum;
	rows[2] = rows[2] / sum;
}
/* Set Column */
void M33::SetColumn(int i, V3 ci){
	rows[0][i] = ci[0];
	rows[1][i] = ci[1];
	rows[2][i] = ci[2];
}

/* Set RotationY */
void M33::SetRotationY(float thetad){
	float rad = thetad*PI / 180;
	rows[0] = V3(cos(rad), 0, sin(rad));
	rows[1] = V3(0, 1, 0);
	rows[2] = V3(-sin(rad), 0, cos(rad));
}

M33 M33::Inverse() {

	M33 ret;
	V3 a = GetColumn(0), b = GetColumn(1), c = GetColumn(2);
	V3 _a = b ^ c; _a = _a / (a * _a);
	V3 _b = c ^ a; _b = _b / (b * _b);
	V3 _c = a ^ b; _c = _c / (c * _c);
	ret[0] = _a;
	ret[1] = _b;
	ret[2] = _c;

	return ret;

}

V3 M33::GetColumn(int i) {

	V3 ret(rows[0][i], rows[1][i], rows[2][i]);
	return ret;

}