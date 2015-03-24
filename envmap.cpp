#include "envmap.h"

#include <iostream>
#include <math.h>


/* Constructor from saved images and camera parameters */
EnvMap::EnvMap(){
	/* Load from saved images with resolution 512x512 */
	face[0]->LoadTiff("1.tif", face[0]);
	face[1]->LoadTiff("2.tif", face[1]);
	face[2]->LoadTiff("3.tif", face[2]);
	face[3]->LoadTiff("4.tif", face[3]);
	face[4]->LoadTiff("5.tif", face[4]);
	face[5]->LoadTiff("6.tif", face[5]);

	// debug
/*	face[0]->w = 512;
	face[0]->h = 512;
	face[1]->w = 512;
	face[1]->h = 512;
	face[2]->w = 512;
	face[2]->h = 512;
	face[3]->w = 512;
	face[3]->h = 512;
	face[4]->w = 512;
	face[4]->h = 512;
	face[5]->w = 512;
	face[5]->h = 512;

	face[0]->Set(0xFF0000FF);
	face[1]->Set(0xFFff779b);
	face[2]->Set(0xFFAA0000);
	face[3]->Set(0xFF0000AA);
	face[4]->Set(0xFF0000BB);
	face[5]->Set(0xFF0000CC);*/



	/* Initialize camera parameters one for each face */
	int w, h;
	float hfov;
	w = 512;
	h = 512;
	hfov = 90.0f;

	/* initialize ppc1 */
	ppc[0] = new PPC(hfov, w, h);
	ppc[0]->a = V3(1.0f, 0.0f, 0.0f);
	ppc[0]->b = V3(0.0f, 4.37114e-008, -1.0f);
	ppc[0]->c = V3(-256.0f, 256.0f, 256.0f);
	vd[0] = ppc[0]->GetVD();

	/* initialize ppc2 */
	ppc[1] = new PPC(hfov, w, h);
	ppc[1]->a = V3(-4.37114e-008, -1.0f, 4.37114e-008);
	ppc[1]->b = V3(0.0f, 4.37114e-008, -1.0f);
	ppc[1]->c = V3(-256.0f, -256.0f, 256.0f);
	vd[1] = ppc[1]->GetVD();

	/* initialize ppc3 */
	ppc[2] = new PPC(hfov, w, h);
	ppc[2]->a = V3(-4.37114e-008, -1.0f, -4.37114e-008);
	ppc[2]->b = V3(0.0f, 4.37114e-008, -1.0f);
	ppc[2]->c = V3(256.0f, 256.0f, 256.0f);
	vd[2] = ppc[2]->GetVD();

	/* initialize ppc4 */
	ppc[3] = new PPC(hfov, w, h);
	ppc[3]->a = V3(-1.0f, 8.74228e-008, 3.82137e-015);
	ppc[3]->b = V3(0.0f, 4.37114e-008, -1.0f);
	ppc[3]->c = V3(256.0f, -256.0f, 256.0f);
	vd[3] = ppc[3]->GetVD();

	/* initialize ppc5 */
	ppc[4] = new PPC(hfov, w, h);
	ppc[4]->a = V3(1.0f, 0.0f, 0.0f);
	ppc[4]->b = V3(0.0f, 1.0f, 8.74288e-008);
	ppc[4]->c = V3(-256.0f, -256.0f, 256.0f);
	vd[4] = ppc[4]->GetVD();

	/* initialize ppc6 */
	ppc[5] = new PPC(hfov, w, h);
	ppc[5]->a = V3(1.0f, 0.0f, 0.0f);
	ppc[5]->b = V3(0.0f, -1.0f, 0.0f);
	ppc[5]->c = V3(-256.0f, 256.0f, -256.0f);
	vd[5] = ppc[5]->GetVD();
}

unsigned int EnvMap::GetColor(V3 rayDirection, V3 eye){
	V3 intersectionPoint;
	V3 planePoint[6];
	unsigned int color = 0;

	for (int i = 0; i < 6; i++){
		planePoint[i] = ppc[i]->GetPointOnImagePlane(1.0f, 1.0f);
		if (eye.IntersectPlaneWithRayWithThisOrigin(rayDirection, vd[i] * -1.0f, planePoint[i], intersectionPoint)){
			int x = intersectionPoint[0];
			int y = intersectionPoint[1];
			int z = intersectionPoint[2];
			int abs_x, abs_y, abs_z;
			abs_x = abs(x);
			abs_y = abs(y);
			abs_z = abs(z);

			int fi;

			int u, v;
			if (abs_x > 256 || abs_y > 256 || abs_z > 256)
				continue;

			if (x == 255 || x == 256){
				u = z + 255;
				v = 255 - y; //256

				if (u > face[2]->w)
					u = face[2]->w - 1;
				else if (u < 0)
					u = 0;
				if (v > face[2]->h)
					v = face[2]->h;
				else if (v < 0)
					v = 0;

				color = face[2]->Get(u, v);
				fi = 2;
			}
			else if (x == -255 || x == -256){
				u = 255 - z; //256
				v = 255 - y; //256

				if (u > face[1]->w)
					u = face[1]->w - 1;
				else if (u < 0)
					u = 0;
				if (v > face[1]->h)
					v = face[1]->h;
				else if (v < 0)
					v = 0;

				color = face[1]->Get(u, v);

				fi = 1;
			}

			if (y == 255 || y == 256){
				u = x + 255;
				v = 255 - z; //256

				if (u > face[4]->w)
					u = face[4]->w - 1;
				else if (u < 0)
					u = 0;
				if (v > face[4]->h)
					v = face[4]->h;
				else if (v < 0)
					v = 0;

				fi = 4;
				color = face[4]->Get(u, v);
			}
			else if (y == -255 || y == -256){
				u = 255 + x;
				v = 255 + z;

				if (u > face[5]->w)
					u = face[5]->w - 1;
				else if (u < 0)
					u = 0;
				if (v > face[5]->h)
					v = face[5]->h;
				else if (v < 0)
					v = 0;

				color = face[5]->Get(u, v);
				fi = 5;
			}

			if (z == 255 || z == 256){
				u = 255 - x; //256
				v = 255 - y; //256

				if (u > face[3]->w)
					u = face[3]->w - 1;
				else if (u < 0)
					u = 0;
				if (v > face[3]->h)
					v = face[3]->h;
				else if (v < 0)
					v = 0;

				color = face[3]->Get(u, v);
				fi = 3;
			}
			else if (z == -255 || z == -256){
				u = x + 255;
				v = 255 - y; //256

				if (u > face[0]->w)
					u = face[0]->w - 1;
				else if (u < 0)
					u = 0;
				if (v > face[0]->h)
					v = face[0]->h;
				else if (v < 0)
					v = 0;

				color = face[0]->Get(u, v);
				fi = 0;

			}
			
			/*if(color == 6)
				cerr << "color:" << color << " face:" << fi << " u:" << u << " v:" << v << endl;*/

			

			
			//cerr << "x: " << x << " y: " << y << " z: " << z << endl;

			/*cerr << "abs_xyz " << abs_x << " " << abs_y << " " << abs_z << endl; 
			
			cerr << "xyz " << x << " " << y << " " << z << endl;
			cerr << " "<< endl;
			
			if (x>0 && y > 0 & z > 0){


			}

			// check for x and find corresponding face
			if (abs_x >0 && abs_x < 512){
				if ((abs_y > 0 && abs_y< 512) && (z > 0 && z < 512)){
					// intersection is in face2
					color = face[1]->Get(x, y);
					//cerr << "face2" << endl;
				}
				// intersection is in face3
				else if ((abs_y>0 && abs_y < 512) && (abs_z > 0 && abs_z < 512)){
					color = face[2]->Get(x, y);
					//cerr << "face3" << endl;
				}
			}

			// check for y and find corresponding faceg
			if (abs_y>0 && abs_y < 512){
				if ((abs_x>0 && abs_x < 512) && (z>0 && z < 512)){
					// intersection is in face6
					color = face[5]->Get(x, y);
					//cerr << "face6" << endl;
				}
				// intersection is in face5
				else if ((abs_x > 0 && abs_x < 512) && (abs_z > 0 && abs_z < 512)){
					color = face[4]->Get(x, y);
					//cerr << "face5" << endl;
				}
			}

			// check for z and find corresponding face
			if (abs_z > 0 && abs_z < 512){
				if ((abs_y > 0 && abs_y< 512) && (x > 0 && x < 512)){
					// intersection is in face1
					color = face[0]->Get(x, y);
					//cerr << "face1" << endl;
				}
				// intersection is in face4
				else if ((abs_y > 0 && abs_y < 512) && (abs_x > 0 && abs_x < 512)){
					color = face[3]->Get(x, y);
					//cerr << "face4" << endl;
				}
			}
			//color = face[i]->Get(x, y);
			// cerr << "color: " << color << endl;*/
		}
		//cerr << "no intersection" << endl;
	}
	return color;
}
