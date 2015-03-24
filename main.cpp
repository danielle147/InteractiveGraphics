
using namespace std;

#include "v3.h"
#include "m33.h"

#include <iostream>

void main() {

  cerr << "HELLO WORLD" << endl;

  V3 v(1.0f, 2.0f, 3.0f);

  M33 m;
  m[0] = v;
  m[1] = v+v;
  m[2] = v+v+v;

  cerr << m << v << endl;
  cerr << "mv = " << m*v << endl;
  return;


  return;

  cerr << "the x component is: " << v[0] << endl;

  v[0] = -10.0f;
  cerr << "the x component is: " << v[0] << endl;

  V3 v1(2.0f, -10.0f, 1.0f);

  cerr << v+v1 << endl;

  V3 s = v + v1;
  cerr << v << endl << v << endl;

}