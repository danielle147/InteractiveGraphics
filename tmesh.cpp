#include "tmesh.h"

#include <fstream>
#include <iostream>
#include <math.h>

/* Constructor - axis aligned box */
TMesh::TMesh(V3 center, V3 dims, unsigned int color) {

	vertsN = 8;
	verts = new V3[vertsN];
	cols = new V3[vertsN];
	normals = new V3[vertsN];
	tcs = new V3[vertsN];
	ttcs = new V3[vertsN];

	int vi = 0;
	verts[vi++] = center + V3(-dims[0] / 2.0f, +dims[1] / 2.0f, +dims[2] / 2.0f);
	verts[vi++] = center + V3(-dims[0] / 2.0f, -dims[1] / 2.0f, +dims[2] / 2.0f);
	verts[vi++] = center + V3(+dims[0] / 2.0f, -dims[1] / 2.0f, +dims[2] / 2.0f);
	verts[vi++] = center + V3(+dims[0] / 2.0f, +dims[1] / 2.0f, +dims[2] / 2.0f);

	verts[vi++] = center + V3(-dims[0] / 2.0f, +dims[1] / 2.0f, -dims[2] / 2.0f);
	verts[vi++] = center + V3(-dims[0] / 2.0f, -dims[1] / 2.0f, -dims[2] / 2.0f);
	verts[vi++] = center + V3(+dims[0] / 2.0f, -dims[1] / 2.0f, -dims[2] / 2.0f);
	verts[vi++] = center + V3(+dims[0] / 2.0f, +dims[1] / 2.0f, -dims[2] / 2.0f);

	for (int vi = 0; vi < vertsN; vi++) {
		cols[vi].SetFromColor(color);
	}

	/* tcs for lookupMode */
	vi = 0;
	tcs[vi++] = V3(1, 1, 0); // 01001011 // 11100100
	tcs[vi++] = V3(1, 0, 0);
	tcs[vi++] = V3(0, 1, 0);
	tcs[vi++] = V3(0, 0, 0);

	tcs[vi++] = V3(1, 0, 0);
	tcs[vi++] = V3(1, 1, 0);
	tcs[vi++] = V3(0, 0, 0);
	tcs[vi++] = V3(0, 1, 0);

	/* tcs for tilingMode */
	vi = 0;
	ttcs[vi++] = V3(0, 0, 0);
	ttcs[vi++] = V3(0, 2, 0);
	ttcs[vi++] = V3(2, 2, 0);
	ttcs[vi++] = V3(2, 0, 0);

	ttcs[vi++] = V3(2, 0, 0);
	ttcs[vi++] = V3(2, 2, 0);
	ttcs[vi++] = V3(0, 2, 0);
	ttcs[vi++] = V3(0, 0, 0);


	trisN = 12;
	tris = new unsigned int[trisN * 3];
	int tri = 0;

	tris[3 * tri + 0] = 0;
	tris[3 * tri + 1] = 1;
	tris[3 * tri + 2] = 2;
	tri++;
	tris[3 * tri + 0] = 2;
	tris[3 * tri + 1] = 3;
	tris[3 * tri + 2] = 0;
	tri++;
	tris[3 * tri + 0] = 3;
	tris[3 * tri + 1] = 2;
	tris[3 * tri + 2] = 6;
	tri++;
	tris[3 * tri + 0] = 6;
	tris[3 * tri + 1] = 7;
	tris[3 * tri + 2] = 3;
	tri++;
	tris[3 * tri + 0] = 4;
	tris[3 * tri + 1] = 0;
	tris[3 * tri + 2] = 3;
	tri++;
	tris[3 * tri + 0] = 3;
	tris[3 * tri + 1] = 7;
	tris[3 * tri + 2] = 4;
	tri++;

	tris[3 * tri + 0] = 5;
	tris[3 * tri + 1] = 6;
	tris[3 * tri + 2] = 2;
	tri++;
	tris[3 * tri + 0] = 2;
	tris[3 * tri + 1] = 1;
	tris[3 * tri + 2] = 5;
	tri++;

	tris[3 * tri + 0] = 4;
	tris[3 * tri + 1] = 7;
	tris[3 * tri + 2] = 6;
	tri++;
	tris[3 * tri + 0] = 6;
	tris[3 * tri + 1] = 5;
	tris[3 * tri + 2] = 4;
	tri++;

	tris[3 * tri + 0] = 4;
	tris[3 * tri + 1] = 5;
	tris[3 * tri + 2] = 1;
	tri++;
	tris[3 * tri + 0] = 1;
	tris[3 * tri + 1] = 0;
	tris[3 * tri + 2] = 4;
	tri++;
}

/* Constructor - load from file */
TMesh::TMesh(char *fname) {

	ifstream ifs(fname, ios::binary);
	if (ifs.fail()) {
		cerr << "INFO: cannot open file: " << fname << endl;
		return;
	}

	ifs.read((char*)&vertsN, sizeof(int));
	char yn;
	ifs.read(&yn, 1); // always xyz
	if (yn != 'y') {
		cerr << "INTERNAL ERROR: there should always be vertex xyz data" << endl;
		return;
	}
	if (verts)
		delete verts;
	verts = new V3[vertsN];

	ifs.read(&yn, 1); // cols 3 floats
	if (cols)
		delete cols;
	cols = 0;
	if (yn == 'y') {
		cols = new V3[vertsN];
	}
	ifs.read(&yn, 1); // normals 3 floats
	if (normals)
		delete normals;
	normals = 0;
	if (yn == 'y') {
		normals = new V3[vertsN];
	}

	ifs.read(&yn, 1); // texture coordinates 2 floats
	float *tcs = 0; // don't have texture coordinates for now
	if (tcs)
		delete tcs;
	tcs = 0;
	if (yn == 'y') {
		tcs = new float[vertsN * 2];
	}

	ifs.read((char*)verts, vertsN * 3 * sizeof(float)); // load verts

	if (cols) {
		ifs.read((char*)cols, vertsN * 3 * sizeof(float)); // load cols
	}

	if (normals)
		ifs.read((char*)normals, vertsN * 3 * sizeof(float)); // load normals

	if (tcs)
		ifs.read((char*)tcs, vertsN * 2 * sizeof(float)); // load texture coordinates

	ifs.read((char*)&trisN, sizeof(int));
	if (tris)
		delete tris;
	tris = new unsigned int[trisN * 3];
	ifs.read((char*)tris, trisN * 3 * sizeof(unsigned int)); // read tiangles

	ifs.close();

	cerr << "INFO: loaded " << vertsN << " verts, " << trisN << " tris from " << endl << "      " << fname << endl;
	cerr << "      xyz " << ((cols) ? "rgb " : "") << ((normals) ? "nxnynz " : "") << ((tcs) ? "tcstct " : "") << endl;

	SetAABB();

}


void TMesh::RenderPoints(PPC *ppc, FrameBuffer *fb, int psize) {

	for (int vi = 0; vi < vertsN; vi++) {
		fb->Draw3DPoint(verts[vi], ppc, psize, cols[vi]);
	}

}

/* Render WireFrame */
void TMesh::RenderWireframe(PPC *ppc, FrameBuffer *fb, unsigned int color) {


	for (int tri = 0; tri < trisN; tri++) {
		int vinds[3];
		vinds[0] = tris[tri * 3 + 0];
		vinds[1] = tris[tri * 3 + 1];
		vinds[2] = tris[tri * 3 + 2];
		for (int vi = 0; vi < 3; vi++) {
			V3 col0, col1;
			if (cols) {
				col0 = cols[vinds[vi]];
				col1 = cols[vinds[(vi + 1) % 3]];
			}
			else {
				col0.SetFromColor(color);
				col1.SetFromColor(color);
			}
			fb->Draw3DSegment(verts[vinds[vi]], verts[vinds[(vi + 1) % 3]], ppc,
				col0, col1);
			//cerr << verts[vinds[vi]] << " and: " << verts[vinds[(vi + 1) % 3]] << endl;
		}
	}
	
}


void TMesh::LoadBin(char *fname) {

	ifstream ifs(fname, ios::binary);
	if (ifs.fail()) {
		cerr << "INFO: cannot open file: " << fname << endl;
		return;
	}

	ifs.read((char*)&vertsN, sizeof(int));
	char yn;
	ifs.read(&yn, 1); // always xyz
	if (yn != 'y') {
		cerr << "INTERNAL ERROR: there should always be vertex xyz data" << endl;
		return;
	}
	if (verts)
		delete verts;
	verts = new V3[vertsN];

	ifs.read(&yn, 1); // cols 3 floats
	if (cols)
		delete cols;
	cols = 0;
	if (yn == 'y') {
		cols = new V3[vertsN];
	}
	ifs.read(&yn, 1); // normals 3 floats
	if (normals)
		delete normals;
	normals = 0;
	if (yn == 'y') {
		normals = new V3[vertsN];
	}

	ifs.read(&yn, 1); // texture coordinates 2 floats
	float *tcs = 0; // don't have texture coordinates for now
	if (tcs)
		delete tcs;
	tcs = 0;
	if (yn == 'y') {
		tcs = new float[vertsN * 2];
	}

	ifs.read((char*)verts, vertsN * 3 * sizeof(float)); // load verts

	if (cols) {
		ifs.read((char*)cols, vertsN * 3 * sizeof(float)); // load cols
	}

	if (normals)
		ifs.read((char*)normals, vertsN * 3 * sizeof(float)); // load normals

	if (tcs)
		ifs.read((char*)tcs, vertsN * 2 * sizeof(float)); // load texture coordinates

	ifs.read((char*)&trisN, sizeof(int));
	if (tris)
		delete tris;
	tris = new unsigned int[trisN * 3];
	ifs.read((char*)tris, trisN * 3 * sizeof(unsigned int)); // read tiangles

	ifs.close();

	cerr << "INFO: loaded " << vertsN << " verts, " << trisN << " tris from " << endl << "      " << fname << endl;
	cerr << "      xyz " << ((cols) ? "rgb " : "") << ((normals) ? "nxnynz " : "") << ((tcs) ? "tcstct " : "") << endl;

	SetAABB();

}

void TMesh::SetAABB() {

	if (!aabb) {
		aabb = new AABB(verts[0]);
	}

	for (int vi = 0; vi < vertsN; vi++)
		aabb->AddPoint(verts[vi]);

	cerr << "TMesh AABB: " << aabb->corners[0] << endl << aabb->corners[1] << endl;

}

void TMesh::Translate(V3 tv) {

	for (int vi = 0; vi < vertsN; vi++)
		verts[vi] = verts[vi] + tv;

}

/* Center */
V3 TMesh::GetCenter() {

	V3 ret = (aabb->corners[0] + aabb->corners[1]) / 2.0f;
	return ret;

}

/* Position */
void TMesh::Position(V3 newCenter) {

	V3 oldCenter = GetCenter();
	Translate(newCenter - oldCenter);
	SetAABB();

}

/* Scale */
void TMesh::ScaleToNewDiagonal(float newDiagonal) {

	float oldDiagonal = (aabb->corners[1] - aabb->corners[0]).Length();
	float sf = newDiagonal / oldDiagonal;
	float Length();
	V3 oldCenter = GetCenter();
	Position(V3(0.0f, 0.0f, 0.0f));
	Scale(sf);
	Position(oldCenter);
	SetAABB();

}

void TMesh::Scale(float scf) {

	for (int vi = 0; vi < vertsN; vi++) {
		verts[vi] = verts[vi] * scf;
	}

	SetAABB();

}

/* Rotate mesh about arbitarary axis theta degrees */
void TMesh::RotateMesh(V3 aO, V3 aD, float thetad){
	for (int vi = 0; vi < vertsN; vi++){
		verts[vi] = verts[vi].RotatePointAboutArbitraryAxis(aO, aD, thetad);
	}
}

void TMesh::RenderFilled(PPC *ppc, PPC *lightView, FrameBuffer *fb,
	unsigned int color, V3 L, float ka, float es, 
	int renderMode, int tilingMode, int lookupMode, FrameBuffer *texture) {
	//texture0->show();
	//texture1->show();
	//texture2->show();
	V3 *pverts = new V3[vertsN];
	bool *proj_flag = new bool[vertsN];
	for (int vi = 0; vi < vertsN; vi++) {
		proj_flag[vi] = ppc->Project(verts[vi], pverts[vi]);
	}

	for (int tri = 0; tri < trisN; tri++) {
		int vinds[3];
		vinds[0] = tris[tri * 3 + 0];
		vinds[1] = tris[tri * 3 + 1];
		vinds[2] = tris[tri * 3 + 2];

		if (!proj_flag[vinds[0]] || !proj_flag[vinds[1]] || !proj_flag[vinds[2]])
			continue;
		for (int vi = 0; vi < 3; vi++){
			V3 col0, col1;
			if (cols) {
				col0 = cols[vinds[vi]];
				col1 = cols[vinds[(vi + 1) % 3]];
			}
			else {
				col0.SetFromColor(color);
				col1.SetFromColor(color);
			}
		}
		// Compute bounding box aabb of projected vertices
		AABB *bbox = new AABB(pverts[vinds[0]]);
		bbox->AddPoint(pverts[vinds[1]]);
		bbox->AddPoint(pverts[vinds[2]]);

		// Clip aabb with frame
		if (fb->IsOutsideFrame(bbox->corners[0][0], bbox->corners[0][1])){
			if (bbox->corners[0][0] < 0)
				bbox->corners[0][0] = 0;
			else if (bbox->corners[0][0]>fb->w)
				bbox->corners[0][0] = fb->w - 1;
			if (bbox->corners[0][1] < 0)
				bbox->corners[0][1] = 0;
			else if (bbox->corners[0][1] > fb->h)
				bbox->corners[0][1] = fb->h - 1;
		}
		else if (fb->IsOutsideFrame(bbox->corners[1][0], bbox->corners[1][1])){
			if (bbox->corners[1][0] < 0)
				bbox->corners[1][0] = 0;
			else if (bbox->corners[1][0]>fb->w)
				bbox->corners[1][0] = fb->w - 1;
			if (bbox->corners[1][1] < 0)
				bbox->corners[1][1] = 0;
			else if (bbox->corners[1][1] > fb->h)
				bbox->corners[1][1] = fb->h - 1;
		}

		// Setup edge equations ee0, ee1, ee2
		float a[3], b[3], c[3];
		float x[3], y[3];
		x[0] = pverts[vinds[0]][0];
		x[1] = pverts[vinds[1]][0];
		x[2] = pverts[vinds[2]][0];
		y[0] = pverts[vinds[0]][1];
		y[1] = pverts[vinds[1]][1];
		y[2] = pverts[vinds[2]][1];

		//ee0
		a[0] = y[1] - y[0];
		b[0] = -x[1] + x[0];
		c[0] = -x[0] * y[1] + y[0] * x[1];
		V3 ee0 = V3(a[0], b[0], c[0]);


		//ee1
		a[1] = y[2] - y[1];
		b[1] = -x[2] + x[1];
		c[1] = -x[1] * y[2] + y[1] * x[2];
		V3 ee1 = V3(a[1], b[1], c[1]);

		//ee2
		a[2] = y[0] - y[2];
		b[2] = -x[0] + x[2];
		c[2] = -x[2] * y[0] + y[2] * x[0];
		V3 ee2 = V3(a[2], b[2], c[2]);

		//Setup rasterization parameter for screen space interpolation (u,v,1)
		M33 rast, rast_inv;
		rast[0] = V3(x[0], y[0], 1);
		rast[1] = V3(x[1], y[1], 1);
		rast[2] = V3(x[2], y[2], 1);
		rast_inv = rast.Inverse();

		float r[3];
		V3 colors[3];
		// Setup screen space linear variation of depth: zABC 
		r[0] = pverts[vinds[0]][2];
		r[1] = pverts[vinds[1]][2];
		r[2] = pverts[vinds[2]][2];
		V3 rZ = V3(r[0], r[1], r[2]);
		V3 zABC = rast_inv*rZ;


		// Get color of three vertex
		if (cols) {
			colors[0].SetFromColor(cols[vinds[0]].GetColor());
			colors[1].SetFromColor(cols[vinds[1]].GetColor());
			colors[2].SetFromColor(cols[vinds[2]].GetColor());
		}
		else {
			colors[0].SetFromColor(color);
			colors[1].SetFromColor(color);
			colors[2].SetFromColor(color);
		}

		// Setup screen space linear variation of red: redABC	
		r[0] = colors[0][0];
		r[1] = colors[1][0];
		r[2] = colors[2][0];
		V3 rRed = V3(r[0], r[1], r[2]);
		V3 redABC = rast_inv*rRed;

		// Setup screen space linear variation of green: greenABC 
		r[0] = colors[0][1];
		r[1] = colors[1][1];
		r[2] = colors[2][1];
		V3 rGreen = V3(r[0], r[1], r[2]);
		V3 greenABC = rast_inv*rGreen;

		// Setup screen space linear variation of blue: blueABC
		r[0] = colors[0][2];
		r[1] = colors[1][2];
		r[2] = colors[2][2];
		V3 rBlue = V3(r[0], r[1], r[2]);
		V3 blueABC = rast_inv*rBlue;

		// Setup screen space linear variation of nx
		r[0] = normals[vinds[0]][0];
		r[1] = normals[vinds[1]][0];
		r[2] = normals[vinds[2]][0];
		V3 rnx = V3(r[0], r[1], r[2]);
		V3 nxABC = rast_inv*rnx;

		// Setup screen space linear variation of ny
		r[0] = normals[vinds[0]][1];
		r[1] = normals[vinds[1]][1];
		r[2] = normals[vinds[2]][1];
		V3 rny = V3(r[0], r[1], r[2]);
		V3 nyABC = rast_inv*rny;

		// Setup screen space linear variation of nz
		r[0] = normals[vinds[0]][2];
		r[1] = normals[vinds[1]][2];
		r[2] = normals[vinds[2]][2];
		V3 rnz = V3(r[0], r[1], r[2]);
		V3 nzABC = rast_inv*rnz;

		// Setup model space interpolation q matrix
		M33 vc, vc_inv, q, abc;
		vc.SetColumn(0, verts[vinds[0]] - ppc->C);
		vc.SetColumn(1, verts[vinds[1]] - ppc->C);
		vc.SetColumn(2, verts[vinds[2]] - ppc->C);
		vc_inv = vc.Inverse();
		abc.SetColumn(0, ppc->a);
		abc.SetColumn(1, ppc->b);
		abc.SetColumn(2, ppc->c);
		q = vc_inv*abc;

		//cerr << "Q: " << q << " vc_inv: " << vc_inv << " abc: " << abc << endl; 

		// Compute D,E,F
		float D, E, F;
		D = q[0][0] + q[1][0] + q[2][0];
		E = q[0][1] + q[1][1] + q[2][1];
		F = q[0][2] + q[1][2] + q[2][2];
		V3 DEF = V3(D, E, F);

		V3 rS, rT;
		if (tilingMode == 0){
			if (tcs){
				r[0] = tcs[vinds[0]][0];
				r[1] = tcs[vinds[1]][0];
				r[2] = tcs[vinds[2]][0];
				rS = V3(r[0], r[1], r[2]);

				r[0] = tcs[vinds[0]][1];
				r[1] = tcs[vinds[1]][1];
				r[2] = tcs[vinds[2]][1];
				rT = V3(r[0], r[1], r[2]);
			}
			else{
				rS = V3(0, 0, 0);
				rT = V3(0, 0, 0);
			}
		}
		else if (tilingMode == 1 || tilingMode == 2){
			if (ttcs){
				r[0] = ttcs[vinds[0]][0];
				r[1] = ttcs[vinds[1]][0];
				r[2] = ttcs[vinds[2]][0];
				rS = V3(r[0], r[1], r[2]);

				r[0] = ttcs[vinds[0]][1];
				r[1] = ttcs[vinds[1]][1];
				r[2] = ttcs[vinds[2]][1];
				rT = V3(r[0], r[1], r[2]);
			}
			else{
				rS = V3(0, 0, 0);
				rT = V3(0, 0, 0);
			}
		}
		else{
			rS = V3(0, 0, 0);
			rT = V3(0, 0, 0);
		}
		
		// Compute sA,sB,sC
		V3 A, B, C;
		A = V3(q[0][0], q[1][0], q[2][0]);
		B = V3(q[0][1], q[1][1], q[2][1]);
		C = V3(q[0][2], q[1][2], q[2][2]);
		
		float aA, bB, cC;
		aA = A*V3(0, 0, 0);
		bB = B*V3(0, 0, 0);
		cC = C*V3(0, 0, 0);
		V3 ABC = V3(aA, bB, cC);
		
		float sA, sB, sC;
		sA = A*rS;
		sB = B*rS;
		sC = C*rS;
		V3 sABC = V3(sA, sB, sC);

		//Compute tA,tB,tC	
		float tA, tB, tC;
		tA = A*rT;
		tB = B*rT;
		tC = C*rT;
		V3 tABC = V3(tA, tB, tC);
	

		// for all rows v of aabb
		// for all columns u of row v

		V3 pv;
		for (int v = (int)(bbox->corners[0][1]+0.5f); v <= (int)(bbox->corners[1][1]-0.5f); v++){
			for (int u = (int)(bbox->corners[0][0]+0.5f); u <= (int)(bbox->corners[1][0]-0.5f); u++){
				// Current pixel is p(u,v)
				// Current pixel vector is pv(u+0.5,v+0.5,1.0)
				pv = V3(u + 0.5, v + 0.5, 1.0);
				if ( pv*ee0 < 0 || pv*ee1 < 0 || pv*ee2 < 0 )
					continue;

				float currz = zABC*pv;
				if (renderMode != 5){
					if (fb->IsOutsideFrame((int)u, (int)v))
						continue;
					if (fb->IsFarther((int)u, (int)v, currz))
						continue;
					fb->SetZ((int)u, (int)v, currz);
				}
				
				float s, t;
				s = (pv*sABC) / (pv*DEF);
				t = (pv*tABC) / (pv*DEF);
				// cerr << "sABC: " << sABC << " tABC : " << tABC << " DEF: " << DEF << endl;
				// cerr << "s: " << s << " t: " << t << endl;


				if (renderMode == 1){	//render color per vertex
					unsigned int c = SSIVColor(pv, redABC, greenABC, blueABC).GetColor();
					// cerr << "u = " << u << " v = " << v << " c=" << c << endl;
					fb->Set(u,v,c);
				}
				else if (renderMode == 2){	//lighting per vertex
					float kd[3];
					float ks[3];
					V3 n0,n1,n2;
					V3 P = ppc->UnProject(V3(u + 0.5, v + 0.5, currz));
					V3 lv = (L - P).Normalized();
					kd[0] = lv * normals[vinds[0]];
					kd[1] = lv * normals[vinds[1]];
					kd[2] = lv * normals[vinds[2]];
					kd[0] = (kd[0] < 0) ? 0 : kd[0];
					kd[1] = (kd[1] < 0) ? 0 : kd[1];
					kd[2] = (kd[2] < 0) ? 0 : kd[2];

					ks[0] = KS(lv, ppc->C, verts[vinds[0]], normals[vinds[0]], es);
					ks[1] = KS(lv, ppc->C, verts[vinds[1]], normals[vinds[1]], es);
					ks[2] = KS(lv, ppc->C, verts[vinds[2]], normals[vinds[2]], es);

					//Setup Screen Space linear variation lredABC
					r[0] = colors[0][0] * (ka + (1 - ka) * kd[0] + ks[0]);
					r[1] = colors[1][0] * (ka + (1 - ka) * kd[1] + ks[1]);
					r[2] = colors[2][0] * (ka + (1 - ka) * kd[2] + ks[2]);
					V3 lrRed = V3(r[0], r[1], r[2]);
					V3 lredABC = rast_inv*lrRed;

					//Setup Screen Space linear variation lgreenABC
					r[0] = colors[0][1] * (ka + (1 - ka) * kd[0] + ks[0]);
					r[1] = colors[1][1] * (ka + (1 - ka) * kd[1] + ks[1]);
					r[2] = colors[2][1] * (ka + (1 - ka) * kd[2] + ks[2]);
					V3 lrGreen = V3(r[0], r[1], r[2]);
					V3 lgreenABC = rast_inv*lrGreen;

					//Setup Screen Space linear variation lblueABC
					r[0] = colors[0][2] * (ka + (1 - ka) * kd[0] + ks[0]);
					r[1] = colors[1][2] * (ka + (1 - ka) * kd[1] + ks[1]);
					r[2] = colors[2][2] * (ka + (1 - ka) * kd[2] + ks[2]);
					V3 lrBlue = V3(r[0], r[1], r[2]);
					V3 lblueABC = rast_inv*lrBlue;

					V3 newColor = SSIVColor(pv, lredABC, lgreenABC, lblueABC);
					fb->Set(u, v, newColor.GetColor());
				}
				else if (renderMode == 3){	//lighting per pixel
					V3 P = ppc->UnProject(V3(u + 0.5, v + 0.5, currz));
					V3 lv = (L - P).Normalized();
					V3 n = SSIN(pv, nxABC, nyABC, nzABC);
					float kd = lv * n;
					kd = (kd < 0) ? 0 : kd;
					float ks = KS(lv, ppc->C, P, n, es);
					V3 origColor = SSIVColor(pv, redABC, greenABC, blueABC);
					V3 newColor = origColor * (ka + (1 - ka)*kd+ks);
					fb->Set(u, v, newColor.GetColor());
				}
				else if (renderMode == 4){	// nearest neighbor texture mapping mode
					//unsigned int textureColor = NearestNeighborLookUp(s, t, texture0);
					//fb->Set(u, v, textureColor);
					TextureMapping(u, v, s, t, fb, texture, lookupMode, tilingMode);
				}
				else if (renderMode == 5){	// draw shadow map
					V3 smCoord;
					int smu, smv;

					//cerr << "u: " << u << " v: " << v << endl;
					smCoord = GetSMCoordinate(ppc, lightView, pv, ABC);
					smu = (int)smCoord[0];
					smv = (int)smCoord[1];
					
					if (smu >= texture->w ) {
						smu = texture->w - 1;
					}
					if (smv >= texture->h) {
						smv = texture->h - 1;
					}
					if (texture->IsFarther(smu, smv, currz)){
						continue;
					}
					texture->SetZ(smu, smv, currz);
					texture->Set(smu, smv, 0xFF000000);
				
				}
				else if (renderMode == 6){	// shadow mapping
					V3 smCoord = GetSMCoordinate(ppc, lightView, pv, ABC);
					float smu, smv, epsilon;
					smu = smCoord[0];
					smv = smCoord[1];
					//cerr << "smu: " << smu << " smv: " << smv << endl;
					epsilon = 0.000001;
					V3 currColor;
					// in shadow
					if ((fb->GetZ((int)u, (int)v) - texture->GetZ((int)smu, (int)smv)) >= epsilon){
						currColor = SSIVColor(pv, redABC, greenABC, blueABC);
						//cerr << "shadow" << endl;
						/*V3 P = ppc->UnProject(V3(u + 0.5, v + 0.5, currz));
						V3 lv = (L - P).Normalized();
						V3 n = SSIN(pv, nxABC, nyABC, nzABC);
						float kd = lv * n;
						kd = (kd < 0) ? 0 : kd;
						currColor = currColor*kd;*/
					}
					else{	// lit
						//cerr << "lit" << endl;
						V3 P = ppc->UnProject(V3(u + 0.5, v + 0.5, currz));
						V3 lv = (L - P).Normalized();
						V3 n = SSIN(pv, nxABC, nyABC, nzABC);
						float kd = lv * n;
						kd = (kd < 0) ? 0 : kd;
						float ks = KS(lv, ppc->C, P, n, es);
						V3 origColor = SSIVColor(pv, redABC, greenABC, blueABC);
						currColor = origColor * (ka + (1 - ka)*kd);
					}
					fb->Set(u, v, currColor.GetColor());
				}
			}
		}
	}
	delete []pverts;
}

// SM1
V3 TMesh::SSIVColor(V3 pv, V3 redABC, V3 greenABC, V3 blueABC){
	// if rendering mode is vertext color interpolation = SM1
	//	ssiRed = redABC * pv;
	float ssiRed = redABC * pv;
	//	ssiGreen = greenABC * pv
	float ssiGreen = greenABC * pv;
	//	ssiBlue = blueABC * pv
	float ssiBlue = blueABC * pv;
	//	FB[p] = (ssiRed, ssiGreen, ssiBlue);
	V3 ret = V3(ssiRed, ssiGreen, ssiBlue);
	return ret;
}

// SM3
V3 TMesh::SSIN(V3 pv, V3 nxABC, V3 nyABC, V3 nzABC){
	// if redering mode is per pixel diffuse lighting = SM3
	V3 n;
	//	n.x = nxABC * pv
	n[0] = nxABC * pv;
	//	n.y = nyABC * pv
	n[1] = nyABC * pv;
	//	n.z = nzABC * pv
	n[2] = nzABC * pv;
	//	normalize n
	n.Normalized();	
	return n;
}
// Compute ks
float TMesh::KS(V3 L, V3 C, V3 P, V3 n, float e){
	float ks;
	//float lr = L*(C - (n*2)*(n*C));
	float lr = (C - P).Normalized() * (P - L).rotateV(P, P + n, 180).Normalized();
	ks = pow(lr, e);
	return ks;
}

void TMesh::SetFromFB(FrameBuffer *fb, PPC *ppc){
	vertsN = fb->w*fb->h;
	verts = new V3[vertsN];
	cols = new V3[vertsN];

	//float z0 = ppc->GetF() / 100.0f;

	for (int v = 0; v < fb->h; v++) {
		for (int u = 0; u < fb->w; u++) {
			int uv = (fb->h - 1 - v)*fb->w + u;
			if (fb->zb[uv] == 0.0f) {
				verts[uv] = V3(FLT_MAX, FLT_MAX, FLT_MAX);
				cols[uv].SetFromColor(fb->Get(u, v));
				continue;
			}
			V3 pp((float)u+0.5f, (float)v+0.5f, fb->zb[uv]);
			//V3 pp((float)u + 0.5f, (float)v + 0.5f, z0);
			verts[uv] = ppc->UnProject(pp);
			cols[uv].SetFromColor(fb->Get(u, v));
		}
	}
}

unsigned int TMesh::NearestNeighborLookUp(float s, float t, FrameBuffer *texture){
	float sw = s*texture->w;
	float th = t*texture->h;
	int x, y;
	
	if (sw - (int)sw <= 0.5)
		x = (int)sw;
	else
		x = (int)sw + 1;
	if (th - (int)th <= 0.5)
		y = (int)th;
	else
		y = (int)th + 1;

	if (x >= texture->w)
		x = texture->w - 1;
	else if (x < 0)
		x = 0;
	if (y >= texture->h)
		y = texture->h - 1;
	else if (y < 0)
		y = 0;

	unsigned int textureColor;
	textureColor = texture->Get(x, y);
	//cerr << "s: " << s << " t: " << t << " x: " << x << " y: " << y << endl;
	return textureColor;
}

unsigned int TMesh::BilinearLookUp(float s, float t, FrameBuffer *texture){	
	float sw = s*texture->w;
	float th = t*texture->h;
	int x = (int) sw;
	int y = (int) th;
	
	
	if (x >= texture->w)
		x = texture->w-1;
	else if (x < 0)
		x = 0;
	if (y >= texture->h)
		y = texture->h-1;
	else if (y < 0)
		y = 0;
		
	float du = sw - x;
	float dv = th - y;
	float odu = 1 - du;
	float odv = 1 - dv;
	unsigned int tex00 = texture->Get(x, y);
	unsigned int tex10 = texture->Get(x + 1, y);
	unsigned int tex01 = texture->Get(x, y + 1);
	unsigned int tex11 = texture->Get(x + 1, y + 1);
	unsigned int textureColor = ((unsigned int)(tex00*odu + tex10*du))*odv + ((unsigned int)(tex01*odu + tex11*du))*dv;
	
	/*static int cnt = 0;
	if (cnt++ == 56){
		cnt = 0;
		cerr << "x: " << x << " y:" << y << " color: " << textureColor << endl;
	}*/
	
	return textureColor; 
}

unsigned int TMesh::TilingMirror(float s, float t, FrameBuffer *texture) {
	float sw, th;
	if (s > 1)
		sw = (1 - (s - (int)s)) * texture->w;
	else if (s < 0) {
		s = -s;
		sw = (1 - (s - (int)s)) * texture->w;
	}
	else
		sw = s * texture->w;

	if (t > 1)
		th = (1 - (t - (int)t)) * texture->h;
	else if (t < 0) {
		t = -t;
		th = (1 - (t - (int)t)) * texture->h;
	}
	else
		th = t * texture->h;

	if (sw >= texture->w)
		sw = texture->w - 1;
	else if (sw < 0)
		sw = 0;
	if (th >= texture->h)
		th = texture->h - 1;
	else if (th < 0)
		th = 0;
	
	unsigned int textureColor = texture->Get(sw, th);
	return textureColor;
}

unsigned int TMesh::TilingRepeat(float s, float t, FrameBuffer *texture) {
	float sw, th;

	if (s > 1)
		sw = (s - (int)s) * texture->w;
	else if (s < 0)
		sw = ((int)(-s + 1) + s) * texture->w;
	else
		sw = s * texture->w;

	if (t > 1)
		th = (t - (int)t) * texture->h;
	else if (t < 0)
		th = ((int)(-t + 1) + t) * texture->h;
	else
		th = t * texture->h;

	if (sw >= texture->w)
		sw = texture->w - 1;
	else if (sw < 0)
		sw = 0;
	if (th >= texture->h)
		th = texture->h - 1;
	else if (th < 0)
		th = 0;

	unsigned int color = texture->Get(sw, th);
	return color;
}

void TMesh::TextureMapping(float u, float v, float s, float t, 
	FrameBuffer *fb, FrameBuffer *texture, int lookupMode, int tilingMode){
	// lookupMode 1 = nearest neighbor; 2 = bilinear
	// tilingMode 0 = no tiling; 1 = repeatition; 2 = mirroring
	unsigned int textureColor;
	if (lookupMode == 1){
		if (tilingMode == 0){
			textureColor = NearestNeighborLookUp(s, t, texture);
		}
		else if (tilingMode == 1){
			textureColor = TilingRepeat(s, t, texture);
		}
		else if (tilingMode == 2){
			textureColor = TilingMirror(s, t, texture);
		}
	}
	else if (lookupMode == 2){
		if (tilingMode == 0){
			textureColor = BilinearLookUp(s, t, texture);
		}
		else if (tilingMode == 1){
			textureColor = TilingRepeat(s, t, texture);
		}
		else if (tilingMode == 2){
			textureColor = TilingMirror(s, t, texture);
		}
	}
	else
		textureColor = 0xFFFFFFFF;
	fb->Set(u, v, textureColor);

}

V3 TMesh::GetSMCoordinate(PPC *ppc, PPC* lightView, V3 pv, V3 ABC){
	M33 labc, labc_inv, q, abc;
	labc.SetColumn(0, lightView->a);
	labc.SetColumn(1, lightView->b);
	labc.SetColumn(2, lightView->c);
	labc_inv = labc.Inverse();

	V3 qq = labc_inv*V3(ppc->C - lightView->C);

	abc.SetColumn(0, ppc->a);
	abc.SetColumn(1, ppc->b);
	abc.SetColumn(2, ppc->c);
	q = labc_inv*abc;

	float inv_w1, u1, v1, u2, v2;
	V3 DEF, GHI, JKL;
	DEF = V3(q[0][0], q[0][1], q[0][2]);
	GHI = V3(q[1][0], q[1][1], q[1][2]);
	JKL = V3(q[2][0], q[2][1], q[2][2]);

	inv_w1 = pv*ABC;
	u1 = pv[0];
	v1 = pv[1];
	u2 = (qq[0] * inv_w1 + (pv*DEF)) / (qq[2] * inv_w1 + (pv*JKL));
	v2 = (qq[1] * inv_w1 + (pv*GHI)) / (qq[2] * inv_w1 + (pv*JKL));

	return V3(u2, v2, 1);
}

/* Get nABC parameters */
M33 TMesh::GetNABC(V3 Vs0, V3 Vs1, V3 Vs2, V3 n0, V3 n1, V3 n2){
	float a[3], b[3], c[3], x[3], y[3], r[3];

	x[0] = Vs0[0];
	x[1] = Vs1[0];
	x[2] = Vs2[0];
	y[0] = Vs0[1];
	y[1] = Vs1[1];
	y[2] = Vs2[1];

	//Setup rasterization parameter for screen space interpolation (u,v,1)
	M33 rast, rast_inv;
	rast[0] = V3(x[0], y[0], 1);
	rast[1] = V3(x[1], y[1], 1);
	rast[2] = V3(x[2], y[2], 1);
	rast_inv = rast.Inverse();

	// Setup screen space linear variation of nx
	r[0] = n0[0];
	r[1] = n1[0];
	r[2] = n2[0];
	V3 rnx = V3(r[0], r[1], r[2]);
	V3 nxABC = rast_inv*rnx;

	// Setup screen space linear variation of ny
	r[0] = n0[1];
	r[1] = n1[1];
	r[2] = n2[1];
	V3 rny = V3(r[0], r[1], r[2]);
	V3 nyABC = rast_inv*rny;

	// Setup screen space linear variation of nz
	r[0] = n0[2];
	r[1] = n1[2];
	r[2] = n2[2];
	V3 rnz = V3(r[0], r[1], r[2]);
	V3 nzABC = rast_inv*rnz;

	M33 ret;
	ret[0] = nxABC;
	ret[1] = nyABC;
	ret[2] = nzABC;
	return ret;
}

/* Get ssi zABC */
V3 TMesh::GetZABC(V3 Vs0, V3 Vs1, V3 Vs2){
	float x[3], y[3], r[3];

	x[0] = Vs0[0];
	x[1] = Vs1[0];
	x[2] = Vs2[0];
	y[0] = Vs0[1];
	y[1] = Vs1[1];
	y[2] = Vs2[1];

	//Setup rasterization parameter for screen space interpolation (u,v,1)
	M33 rast, rast_inv;
	rast[0] = V3(x[0], y[0], 1);
	rast[1] = V3(x[1], y[1], 1);
	rast[2] = V3(x[2], y[2], 1);
	rast_inv = rast.Inverse();
	
	// Setup screen space linear variation of depth: zABC 
	r[0] = Vs0[2];
	r[1] = Vs1[2];
	r[2] = Vs2[2];
	V3 rZ = V3(r[0], r[1], r[2]);
	V3 zABC = rast_inv*rZ;

	return zABC;
}

void TMesh::RenderHW() {
	
	boolean colorFlag = false;
	boolean texFlag = false;
	
	if (isWF){
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		colorFlag = true;
		texFlag = false;
	}
	else if (isTM){
		colorFlag = false;
		texFlag = true;
	}
	else{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		colorFlag = true;
		texFlag = false;
	}
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	if (texFlag == true)
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	if (colorFlag == true)
		glEnableClientState(GL_COLOR_ARRAY);
	
	glVertexPointer(3, GL_FLOAT, 0, (float*)verts);
	glNormalPointer(GL_FLOAT, 0, (float*)normals);
	if (texFlag == true)
		glTexCoordPointer(2, GL_FLOAT, 0, (float*)tcs);
	if (colorFlag == true)
		glColorPointer(3, GL_FLOAT, 0, (float*)cols);

	glDrawElements(GL_TRIANGLES, 3 * trisN, GL_UNSIGNED_INT, tris);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	if (texFlag == true)
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	if (colorFlag == true)
		glDisableClientState(GL_COLOR_ARRAY);
}

