#include "scene.h"
#include "m33.h"
#include "ppc.h"
#include "tmesh.h"
#include "libraries\include\SOIL\SOIL.h"
#include "time.h"


#include <sstream>
#include <iostream>
#include <list>

using namespace std;

Scene *scene;
int lookupMode = 1;
int renderMode = 1;
int tilingMode = 0;
int tmi = 1;


Scene::Scene() {

  // gpu programming interface
  cgi = new CGInterface();
  soi = new ShaderOneInterface();

  hwInit = false;

  gui = new GUI();
  gui->show();

  int u0 = 20;
  int v0 = 50;
  int sci = 2;
  int w = sci*320; //320 256
  int h = sci*240; //240 256
  
  // create SW Framebuffer, HW Framebuffer, and shadow map
  fb = new FrameBuffer(u0, v0, w, h);
  fb->label("SW Framebuffer");
  fb->show();
  gui->uiw->position(fb->w+20+20, 50);

 // hwfb = new FrameBuffer(fb->w + u0 + 20, v0, fb->w, fb->h);
  hwfb = new FrameBuffer(fb->w+u0+20, v0, fb->w, fb->h);
  hwfb->label("HW Framebuffer");
  hwfb->isHW = true;
  //hwfb->show();

  shadowMap = new FrameBuffer(w, h, 512, 512);

  // create ppc and lightView ppc
  float hfov = 45.0f; //55.0f
 
  ppc = new PPC(hfov, w, h);
  lightView = new PPC(2 * hfov, w, h);

  // load textures here
  fb->LoadTiff("texture3.tif", texture);
  fb->LoadTiff("texture1.tif", texture1);
  fb->LoadTiff("texture2.tif", texture2);
  fb->LoadTiff("ground.tif", ground);

  fb->Set(0xFFFFFFFF);

  // create tmeshes
  tmeshesN = 11;
  tmeshes = new TMesh*[tmeshesN];

  V3 center(0.0f, -150.0f, -200.0f); //7,-10,-65 //0,-150,-200
  V3 dims(200.0f, 200.0f, 220.0f); //20,10,45 //180,180,180
  unsigned int color = 0xFF00FF00;

  // initial box
  tmeshes[0] = new TMesh(center, dims, color);
  tmeshes[0]->enabled = false;
  tmeshes[0]->isWF = false;
  tmeshes[0]->isTM = true;
  tmeshes[0]->RotateMesh(center, ppc->a,-90);

  // initial teapot
  tmeshes[1] = new TMesh();
  tmeshes[1]->LoadBin("geometry/teapot1k.bin");	
  V3 newCenter = V3(0.0f, 0.0f, -100.0f); //0,0,-150
  tmeshes[1]->Position(newCenter);
  tmeshes[1]->enabled = false;
  tmeshes[1]->isWF = false;

  tmeshes[2] = new TMesh();
  tmeshes[2]->enabled = false;

  // comparing teapot 1 for lighting
  tmeshes[3] = new TMesh();
  tmeshes[3]->LoadBin("geometry/teapot1k.bin");
  tmeshes[3]->enabled = false;

  // comparing teapot 2 for lighting
  tmeshes[4] = new TMesh();
  tmeshes[4]->LoadBin("geometry/teapot1k.bin");
  tmeshes[4]->enabled = false;

  // comparing box 1 for texture mapping
  center = V3(0, -10, -100);
  tmeshes[5] = new TMesh(center, dims, color);
  tmeshes[5]->enabled = false;

  // comparing box 2 for texture mapping
  center = V3(30, -10, -100);
  tmeshes[6] = new TMesh(center, dims, color);	//	comparing teapot2
  tmeshes[6]->enabled = false;

  // back teapot for shadow mapping
  tmeshes[7] = new TMesh();
  tmeshes[7]->LoadBin("geometry/teapot1k.bin");
  tmeshes[7]->enabled = false;

  // front teapot for shadow mapping
  tmeshes[8] = new TMesh();
  tmeshes[8]->LoadBin("geometry/teapot1k.bin");
  tmeshes[8]->enabled = false;

  // table for shadow mapping
  center = V3(0, -80, -250);
  dims = V3(220, 100, 100);
  color = 0xFF000000;
  tmeshes[9] = new TMesh(center, dims, color);
  tmeshes[9]->enabled = false;

  // auditorium model
  tmeshes[10] = new TMesh();
  tmeshes[10]->LoadBin("geometry/auditorium.bin");
  V3 newCenterForAuditorium = V3(0.0f, 0.0f, 0.0f); //0,0,-150
  tmeshes[10]->Position(newCenterForAuditorium);
  tmeshes[10]->enabled = false;

  //CreatePPCPath();
  
  animationRadius = (tmeshes[1]->aabb->corners[1] - tmeshes[1]->aabb->corners[0]).Length() * 0.5f;
  animationCenter = tmeshes[1]->GetCenter();
  animationFraction = 0.0f;
  light = tmeshes[1]->GetCenter() + V3(0.0f, tmeshes[1]->aabb->corners[1][1] - tmeshes[1]->aabb->corners[0][1], 0.0f);

  // load environment map
  em = new EnvMap();
  isEnvMap = false;

  // set value for panorama
  isPanorama = true;
  
  Render();
  //fb->redraw();
}

void Scene::SetTexture(){

	// set texture
	
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	int width, height;
	unsigned char* image = SOIL_load_image("checker.jpg", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex);

	glBegin(GL_QUADS);
	glTexCoord2d(0.0, 0.0); 
	glTexCoord2d(1.0, 0.0);
	glTexCoord2d(1.0, 1.0);
	glTexCoord2d(0.0, 1.0);

	glTexCoord2d(0.0, 0.0);
	glTexCoord2d(1.0, 0.0);
	glTexCoord2d(1.0, 1.0);
	glTexCoord2d(0.0, 1.0);
	glEnd();
}

void Scene::RenderHW() {

	if (!hwInit) {
		glEnable(GL_DEPTH_TEST);
		hwInit = true;
		// initialize textures
	}

	// initialize GPU programming interfaces
	if (cgi->needInit) {
		cgi->PerSessionInit();
		soi->PerSessionInit(cgi);
	}

	// clear the framebuffer
	glClearColor(255.0f, 255.0f, 255.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// set the view
	float nearz = 1.0f;
	float farz = 1000.0f;
	ppc->SetIntrinsicsHW(nearz, farz);
	ppc->SetExtrinsicsHW();

	// draw the texture mapped quad without shaders
	for (int tmi = 0; tmi < tmeshesN; tmi++) {
		if (!tmeshes[tmi]->enabled)
			continue;
		if (tmeshes[tmi]->isTM){
			SetTexture();
			tmeshes[tmi]->RenderHW();
		}
	}

	// enable shaders when rendering with shaders
	bool renderWithShaders = true;
	if (renderWithShaders) {
		soi->PerFrameInit();
		soi->BindPrograms();
		cgi->EnableProfiles();
	}

	// draw the tmeshes
	for (int tmi = 0; tmi < tmeshesN; tmi++) {
		if (!tmeshes[tmi]->enabled)
			continue;
		if (tmeshes[tmi]->isTM){
			SetTexture();
		}
		tmeshes[tmi]->RenderHW();
	}

	// disable shaders when rendering with shaders
	if (renderWithShaders) {
		soi->PerFrameDisable();
		cgi->DisableProfiles();
	}

	glPointSize(5.0f);
	glBegin(GL_POINTS);
	glVertex3fv((float*)&light);
	glEnd();
}

void Scene::RayTrace() {

	for (int v = 0; v < fb->h; v++) {

		for (int u = 0; u < fb->w; u++) {
			fb->Set(u, v, 0xFFAAAAAA);
			fb->SetZ(u, v, FLT_MAX);
			V3 ray = (ppc->c + ppc->a*((float)u + .5f) + ppc->b*((float)v + .5f)).UnitVector();
			for (int tmi = 0; tmi < tmeshesN; tmi++) {
				if (!tmeshes[tmi]->enabled)
					continue;
				for (int tri = 0; tri < tmeshes[tmi]->trisN; tri++) {
					V3 Vs[3];
					Vs[0] = tmeshes[tmi]->verts[tmeshes[tmi]->tris[3 * tri + 0]];
					Vs[1] = tmeshes[tmi]->verts[tmeshes[tmi]->tris[3 * tri + 1]];
					Vs[2] = tmeshes[tmi]->verts[tmeshes[tmi]->tris[3 * tri + 2]];
					V3 bgt = ppc->C.IntersectRayWithTriangleWithThisOrigin(
						ray, Vs);
					if (bgt[2] < 0.0f || bgt[0] < 0.0f || bgt[1] < 0.0f ||
						bgt[0] + bgt[1] > 1.0f)
						continue;
					if (fb->zb[(fb->h - 1 - v)*fb->w + u] < bgt[2])
						continue;
					fb->SetZ(u, v, bgt[2]);
					float alpha = 1.0f - bgt[0] - bgt[1];
					float beta = bgt[0];
					float gamma = bgt[1];
					V3 Cs[3];
					Cs[0] = tmeshes[tmi]->cols[tmeshes[tmi]->tris[3 * tri + 0]];
					Cs[1] = tmeshes[tmi]->cols[tmeshes[tmi]->tris[3 * tri + 1]];
					Cs[2] = tmeshes[tmi]->cols[tmeshes[tmi]->tris[3 * tri + 2]];
					V3 color = Cs[0] * alpha + Cs[1] * beta + Cs[2] * gamma;
					fb->Set(u, v, color.GetColor());
				}
			}
		}
		fb->redraw();
		Fl::check();
	}

}

void Scene::Render() {

	/*bool rayTracing = true;
	if (rayTracing){
		RayTrace();
		fb->redraw();
		return;
	}*/
	bool hwOnly = false;
	if (hwfb) {
		hwfb->redraw();
		if (hwOnly)
			return;
	}

	unsigned int color = 0xFF0000FF;
	V3 lightPoint(tmeshes[1]->GetCenter() + V3(0.0f, 0.0f, 100.0f));
	float ka = .0f;

	fb->Clear(0xFFFFFFFF, 0.0f);
	//EMSW();
	for (int tmi = 0; tmi < tmeshesN; tmi++) {
		if (!tmeshes[tmi]->enabled)
			continue;
		if (tmeshes[tmi]->trisN == 0)
			tmeshes[tmi]->RenderPoints(ppc, fb, 1);
		else {
			//      tmeshes[tmi]->RenderWireframe(ppc, fb, 0xFF000000);
			int renderMode = 1;
			tmeshes[tmi]->RenderFilled(ppc, ppc, fb, color, lightPoint, ka, 0.5, renderMode, tilingMode, lookupMode, texture);
		}
	}

	// render light
	fb->Draw3DPoint(light, ppc, 5, V3(0.5f, 0.5f, 0.0f));

	fb->redraw();

}

/* Read filename */
const char* Scene::ReadFile(){
	//cerr << "The file name is: " << gui->Filename->value() << endl;
	cerr << "FILE OPEN: " << gui->Filename->value() << endl;
	return gui->Filename->value();
}

/* Load file button */
void Scene::LoadFile(){
	FrameBuffer *tiffImage;
	const char* file = ReadFile();
	if (file=="")
		cerr << "FILENAME EMPTY!" << endl;
	else{
		unsigned int ret = fb->LoadTiff(file, fb);
		cerr << "LoadTiff return: " << ret << endl;
		fb->show();	
	}
}

/* SaveTo filename */
const char* Scene::SaveTo(){
	cerr << "FILE SAVE TO: " << gui->SaveTo->value() << endl;
	return gui->SaveTo->value();
}

/* Save File */
void Scene::SaveFile(){
	fb->SaveTiff(SaveTo());
}

/* Read adjust percentage from input */
const char* Scene::BrightPercent(){
	cerr << "ADJUST PERCENT: " << gui->BrightPercent->value() << endl;
	return gui->BrightPercent->value();
}

/* Adjust Brightness */
void Scene::AdjustBrightness(){
	const char* factor = BrightPercent();
	stringstream val;
	int percent;
	val << factor;
	val >> percent;

	cerr << "FACTOR: " << atoi(factor) << endl;
	fb->AdjustBrightness(percent);
	Render();
}

/* Draw my name - XXL */
void Scene::DrawName(){
	//fb->DrawCircle(fb->w/2, fb->h/2, 50.0f, 5, 0xFF000000);
	/* Location to draw first letter */
	for (float i = fb->w  / 8; i < fb->w * 3 / 8; i++){
		fb->DrawCircle(i, 2 * i + fb->h / 2 - fb->w/2, 4.0f, 6.0f, 0xFF000000);
	}
	for (float i = fb->w  / 8; i < fb->w * 3 / 8; i++){
		fb->DrawCircle(i, -2 * i + fb->h / 2 + fb->w/2, 4.0f, 6.0f, 0xFF000000);
	}
	/* Location to draw middle letter*/
	for (float i = fb->w*3  / 8; i < fb->w * 5 / 8; i++){
		fb->DrawCircle(i, 2 * i + fb->h / 2 - fb->w , 4.0f, 6.0f, 0xFF000000);
	}
	for (float i = fb->w * 3 / 8; i < fb->w * 5 / 8; i++){
		fb->DrawCircle(i, -2 * i + fb->h / 2 + fb->w, 4.0f, 6.0f, 0xFF000000);
	}
	/* Location to draw last letter */
	for (float i = fb->h*3 / 16; i < fb->h *13 / 16; i++){
		fb->DrawCircle(fb->w * 5 / 8, i, 4.0f, 6.0f, 0xFF000000);
	}
	for (int i = fb->w * 5 / 8; i < fb->w * 7 / 8; i++){
		fb->DrawCircle(i, fb->h * 13 / 16, 4.0f, 6.0f, 0xFF000000);
	}
	fb->redraw();
}

/* Edge detection */
void Scene::EdgeDetect(){
	M33 edgeDetect;
	edgeDetect[0] = V3(-1.0f, -1.0f, -1.0f);
	edgeDetect[1] = V3(-1.0f, 8.0f, -1.0f);
	edgeDetect[2] = V3(-1.0f, -1.0f, -1.0f);
	//edgeDetect.Normalize();

	FrameBuffer *EdgeDetectedImg = 0;
	fb->EdgeDetect(edgeDetect,EdgeDetectedImg);
	EdgeDetectedImg->show();
	//Render();
}

/* Visualize Camera */
void Scene::Visualize(){
	float hfov = 55.0f;
	float w = 320;
	float h = 240;
	PPC ppc0(hfov, w, h);
	PPC ppc1(2*hfov, 2*w, 2*h);
	ppc1.C = V3(0, 0, h);
	ppc0.Visualize(&ppc1, fb, 0xFF000000);
	Render();
}

/* Up */
void Scene::Up(){
	V3 L = V3(0, 0, 0);
	float step = 1.0f;
	ppc->UpDownTranslate(step);
	ppc->SaveToTxt("GUICameraPath.txt");
	Render();
}

/* Down */
void Scene::Down(){
	V3 L = V3(0, 0, 0);
	float step = -1.0f;
	ppc->UpDownTranslate(step);
	ppc->SaveToTxt("GUICameraPath.txt");
	Render();
}

/* Left */
void Scene::Left(){
	V3 L = V3(0, 0, 0);
	float step = -5.0f;
	ppc->LeftRightTranslate(step);
	ppc->SaveToTxt("GUICameraPath.txt");
	if (isPanorama)
		Panorama();
	else
		Render();
}

/* Right */
void Scene::Right(){
	V3 L = V3(0, 0, 0);
	float step = 5.0f;
	ppc->LeftRightTranslate(step);
	ppc->SaveToTxt("GUICameraPath.txt");
	if (isPanorama)
		Panorama();
	else
		Render();
}

/* Front */
void Scene::Front(){
	V3 L = V3(0, 0, 0);
	float step = 1.0f;
	ppc->FrontBackTranslate(step);
	ppc->SaveToTxt("GUICameraPath.txt");
	Render();
}

/* Back */
void Scene::Back(){
	V3 L = V3(0, 0, 0);
	float step = -1.0f;
	ppc->FrontBackTranslate(step);
	ppc->SaveToTxt("GUICameraPath.txt");
	Render();
}

/* Pan Left */
void Scene::PanLeft(){
	V3 L = V3(0, 0, 0);
	float deg = 10.0f;
	ppc->PanRotate(deg);
	ppc->SaveToTxt("GUICameraPath.txt");
	cerr << "C: " << ppc->C << " a: " << ppc->a << " b: " << ppc->b << " c: " << ppc->c << endl;
	if (isEnvMap)
		EMSW();
	else if (isPanorama)
		Panorama();
	else
		Render();
}

/* Pan Right */
void Scene::PanRight(){
	V3 L = V3(0, 0, 0);
	float deg = -10.0f;
	ppc->PanRotate(deg);
	ppc->SaveToTxt("GUICameraPath.txt");
	cerr << "C: " << ppc->C << " a: " << ppc->a << " b: " << ppc->b << " c: " << ppc->c << endl;
	if (isEnvMap)
		EMSW();
	else if (isPanorama)
		Panorama();
	else
		Render();
}

/* Tilt Up */
void Scene::TiltUp(){
	V3 L = V3(0, 0, 0);
	float deg = 5.0f;
	ppc->TiltRotate(deg);
	ppc->SaveToTxt("GUICameraPath.txt");
	cerr << "C: " << ppc->C << " a: " << ppc->a << " b: " << ppc->b << " c: " << ppc->c << endl;

	if (isEnvMap)
		EMSW();
	else
		Render();
}

/* Tilt Down */
void Scene::TiltDown(){
	V3 L = V3(0, 0, 0);
	float deg = -5.0f;
	ppc->TiltRotate(deg);
	ppc->SaveToTxt("GUICameraPath.txt");
	cerr << "C: " << ppc->C << " a: " << ppc->a << " b: " << ppc->b << " c: " << ppc->c << endl;
	if (isEnvMap)
		EMSW();
	else
		Render();
}

/* Roll Left */
void Scene::RollLeft(){
	V3 L = V3(0, 0, 0);
	float deg = -5.0f;
	ppc->RollRotate(deg); 
	ppc->SaveToTxt("GUICameraPath.txt");
	cerr << "C: " << ppc->C << " a: " << ppc->a << " b: " << ppc->b << " c: " << ppc->c << endl;
	if (isEnvMap)
		EMSW();
	else
		Render();
}

/* Roll Right */
void Scene::RollRight(){
	V3 L = V3(0, 0, 0);
	float deg = 5.0f;
	ppc->RollRotate(deg);
	ppc->SaveToTxt("GUICameraPath.txt");
	cerr << "C: " << ppc->C << " a: " << ppc->a << " b: " << ppc->b << " c: " << ppc->c << endl;
	if (isEnvMap)
		EMSW();
	else
		Render();
}

/* Zoom in */
void Scene::ZoomIn(){
	V3 L = V3(0, 0, 0);
	float zoom = 1.1f;
	ppc->Zoom(zoom);
	ppc->SaveToTxt("GUICameraPath.txt");
	Render();
}

/* Zoom out */
void Scene::ZoomOut(){
	V3 L = V3(0, 0, 0);
	float zoom = 0.9f;
	ppc->Zoom(zoom);
	ppc->SaveToTxt("GUICameraPath.txt");
	Render();
}

/* Play */
void Scene::Play(){

	V3 L = V3(0, 0, 0);
	tmi = 1;
	renderMode = 1;

	list<PPC> camera_path = ppc->LoadCameraPathFromTxt("GUICameraPath.txt");
	int fi = 0;
	for (list<PPC>::iterator it = camera_path.begin(); it != camera_path.end(); it++){
		fb->Clear(0xFFFFFFFF, 0.0f);
		PPC point = *it;
		tmeshes[tmi]->RenderFilled(&point, ppc, fb, 0xFF0000FF, L, 0.2, 0.5, renderMode, tilingMode, lookupMode, texture);
		fb->redraw();
		Fl::check();
		string fname = "movies/" + to_string(fi) + ".tif";
		fb->SaveTiff(fname.c_str());
		fi++;
	}
	
}

/* Play HW */
void Scene::PlayHW(){

	/*int frame = 0, time, timebase = 0;
	float fps = 0;

	frame++;
	//time = glutGet(GLUT_ELAPSED_TIME);

	if (time - timebase > 1000) {
		fps = frame*1000.0 / (time - timebase);
		timebase = time;
		frame = 0;
	}*/

	list<PPC> camera_path1 = ppc->LoadCameraPathFromTxt("GUICameraPath1.txt");
	tmeshes[1]->enabled = true;
	tmeshes[1]->isWF = true;
	tmeshes[0]->enabled = false;
	for (list<PPC>::iterator it = camera_path1.begin(); it != camera_path1.end(); it++){
		PPC point = *it;
		ppc->SetByInterpolation(&point, &point, 1);
		hwfb->redraw();
		Sleep(20);
		Fl::check();
	}
	tmeshes[1]->isWF = false;
	tmeshes[0]->enabled = false;
	list<PPC> camera_path2 = ppc->LoadCameraPathFromTxt("GUICameraPath2.txt");
	for (list<PPC>::iterator it = camera_path2.begin(); it != camera_path2.end(); it++){
		PPC point = *it;
		ppc->SetByInterpolation(&point, &point, 1);
		hwfb->redraw();
		Sleep(20);
		Fl::check();
	}
	tmeshes[1]->enabled = false;
	tmeshes[0]->enabled = true;
	tmeshes[0]->isTM = true;
	list<PPC> camera_path3 = ppc->LoadCameraPathFromTxt("GUICameraPath3.txt");
	for (list<PPC>::iterator it = camera_path3.begin(); it != camera_path3.end(); it++){
		PPC point = *it;
		ppc->SetByInterpolation(&point, &point, 1);
		hwfb->redraw();
		Sleep(20);
		Fl::check();
	}
	
	//glutPostRedisplay();
		 // in c time
		//time_t timer;
		//time(&timer);
		//struct tm y2k;
		//double seconds;

		//seconds = difftime(timer, mktime(&y2k));
		//float fps = 1 / seconds;
		//cerr << fps << endl;
		//glRasterPos2d(5, 5);

		//char *display = "FPS: ";
		///int displayInt = atoi(display);
		//glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_10, displayInt);
}

/* SM2 Play */
void Scene::SM2Play(){
	int renderMode = 2;
	V3 L = V3(-300.0f, 30.0f, -80.0f);
	V3 step = V3(10, 0, 0);
	//int fi = 150;
	for (int i = 0; i < 50; i++){
		fb->Clear(0xFFFFFFFF, 0.0f);
		tmeshes[1]->RenderFilled(ppc, ppc,fb, 0xFF0000FF, L = L + step, 0.2,600,renderMode,tilingMode,lookupMode,texture);
		fb->redraw();
		Fl::check();
		//string fname = "SM2/" + to_string(fi) + ".tif";
		//fb->SaveTiff(fname.c_str());
		//fi++;
	}

}

/* SM3 Play */
void Scene::SM3Play(){
	int renderMode = 3;
	V3 L = V3(-300.0f, 30.0f, -80.0f);
	V3 step = V3(10, 0, 0);
	//int fi = 200;
	for (int i = 0; i < 50; i++){
		fb->Clear(0xFFFFFFFF, 0.0f);
		tmeshes[1]->RenderFilled(ppc, ppc,fb, 0xFF0000FF, L = L + step, 0.2,600, renderMode,tilingMode,lookupMode,texture);
		fb->redraw();
		Fl::check();
		//string fname = "SM3/" + to_string(fi) + ".tif";
		//fb->SaveTiff(fname.c_str());
		//fi++;
	}
}

/* SM2SM3 Play */
void Scene::SM2SM3(){
	V3 Lsm2 = V3(-320, 30, -80);
	V3 Lsm3 = V3(-160, 30, -80);
	V3 step = V3(10, 0, 0);

	V3 p1 = V3(0, -240, -300);
	V3 p2 = V3(0, 240, -300);
	V3 c;
	c.SetFromColor(0xFF000000);

	V3 newC = V3(-80.0f, 0.0f, -300.0f);
	tmeshes[3]->Position(newC);

	newC = V3(80.0f, 0.0f, -300.0f);
	tmeshes[4]->Position(newC);

	//int fi = 250;
	for (int i = 0; i < 50; i++){
		fb->Clear(0xFFFFFFFF, 0.0f);
		tmeshes[3]->RenderFilled(ppc,ppc, fb, 0xFF0000FF, Lsm2 = Lsm2+step, 0.2,5, 2,0,0,texture);
		tmeshes[4]->RenderFilled(ppc, ppc,fb, 0xFF0000FF, Lsm3 = Lsm3+step, 0.2,5, 3,0,0,texture);
		fb->Draw3DSegment(p1, p2, ppc, c, c);
		fb->redraw();
		Fl::check();
		//string fname = "SM2SM3/" + to_string(fi) + ".tif";
		//fb->SaveTiff(fname.c_str());
		//fi++;
	}
}

/* Texture mapping display */
void Scene::Display(){
	renderMode = 4;
	tilingMode = 0;
	lookupMode = 1;

	V3 L = V3(0, 0, 0);
	float step;
	fb->Clear(0xFFFFFFFF, 0.0f);
	step = -1;
	for (int i = 0; i < 15; i++){
		ppc->UpDownTranslate(step);
		fb->Clear(0xFFFFFFFF, 0);
		tmeshes[5]->RenderFilled(ppc, ppc,fb, 0xFF0000FF, L, 0, 0, renderMode, 2, lookupMode, texture2);
		tmeshes[0]->RenderFilled(ppc, ppc,fb, 0xFF0000FF, L, 0, 0, renderMode, tilingMode, lookupMode, texture);
		tmeshes[6]->RenderFilled(ppc, ppc,fb, 0xFF0000FF, L, 0, 0, renderMode, tilingMode, lookupMode, texture1);
		fb->redraw();
		Fl::check();
	}

	fb->redraw();
	for (int i = 0; i < 15; i++)
		ppc->UpDownTranslate(1.0f);

}

/* Save Camera Path */
void Scene::CreatePPCPath(){

	/* Zoom-in section */
	float zoom = 1.1f;
	for (int i = 0; i < 5; i++){
		ppc->Zoom(zoom);
		ppc->SaveToTxt("CameraPath.txt");
	}
	/* left section */
	float step = -1.0f;
	for (int i = 0; i < 20; i++){
		ppc->LeftRightTranslate(step);
		ppc->SaveToTxt("CameraPath.txt");
	}
	/* right section */
	step = 1.0f;
	for (int i = 0; i < 40; i++){
		ppc->LeftRightTranslate(step);
		ppc->SaveToTxt("CameraPath.txt");
	}
	/* left section */
	step = -1.0f;
	for (int i = 0; i < 20; i++){
		ppc->LeftRightTranslate(step);
		ppc->SaveToTxt("CameraPath.txt");
	}

	


	//float step;
	/* Up section */
/*	step = 1.0f;
	for (int i = 0; i < 15; i++){
		ppc->UpDownTranslate(step);
		ppc->SaveToTxt("CameraPath.txt");
	}*/

	/* Down section */
	//step = -1.0f;
/*	for (int i = 0; i < 60; i++){
		ppc->UpDownTranslate(step);
		ppc->SaveToTxt("CameraPath.txt");
	}
	/* Up section */
/*	step = 1.0f;
	for (int i = 0; i < 40; i++){
		ppc->UpDownTranslate(step);
		ppc->SaveToTxt("CameraPath.txt");
	}
	
	

	/* Zoom-out section */
	/*zoom = 0.9f;
	for (int i = 0; i < 10; i++){
		ppc->Zoom(zoom);
		ppc->SaveToTxt("CameraPath.txt");
	}*/
	

	


	/* Back section */
/*	for (int i = 0; i < 30; i++){
		ppc->FrontBackTranslate(step);
		ppc->SaveToTxt("CameraPath.txt");
	}*/
	/* Front section */ 
/*	step = 1.0f;
	for (int i = 0; i < 30; i++){
		ppc->FrontBackTranslate(step);
		ppc->SaveToTxt("CameraPath.txt");
	}*/

	/* Roll section */
	/*float deg = 30.0f;
	for (int i = 0; i < 12; i++){
		ppc->RollRotate(deg);
		ppc->SaveToTxt("CameraPath.txt");
	}
	deg = -30.0f;
	for (int i = 0; i < 12; i++){
		ppc->RollRotate(deg);
		ppc->SaveToTxt("CameraPath.txt");
	}

	zoom = 0.93;
	// Zoom out before tilt
	for (int i = 0; i < 10; i++){
		ppc->Zoom(zoom);
		ppc->SaveToTxt("CameraPath.txt");
	}*/
	
	//deg = 2.0f;
	/* Tilt section */
/*	for (int i = 0; i < 24; i++){
		ppc->TiltRotate(deg);
		ppc->SaveToTxt("CameraPath.txt");
	}
	deg = -2.0f;
	for (int i = 0; i < 24; i++){
		ppc->TiltRotate(deg);
		ppc->SaveToTxt("CameraPath.txt");
	}*/

	/* Pan section */
/*	for (int i = 0; i < 24; i++){
		ppc->PanRotate(deg);
		ppc->SaveToTxt("CameraPath.txt");
	}
	deg = 2.0f;
	for (int i = 0; i < 24; i++){
		ppc->PanRotate(deg);
		ppc->SaveToTxt("CameraPath.txt");
	}*/
	
}

/* read pointLightx */
const char* Scene::pointLightx(){
	return gui->pointLightx->value();
}

/* read pointLighty */
const char* Scene::pointLighty(){
	return gui->pointLighty->value();
}

/* read pointLightz */
const char* Scene::pointLightz(){
	return gui->pointLightz->value();
}

/* read ka */
const char* Scene::Ka(){
	return gui->ka->value();
}

/* read ks */
const char* Scene::Es(){
	return gui->es->value();
}

/* Draw Point Light */
void Scene::DrawPointLight(){
	float x = atof(pointLightx());
	float y = atof(pointLighty());
	float z = atof(pointLightz());
	V3 pt = V3(x, y, z);
	FrameBuffer *pointLightImg = new FrameBuffer(20,50,640,480);
	pointLightImg->Set(0xFF000000);
	pointLightImg->label("Point Light");
	pointLightImg->DrawPointLight(ppc, pt);
	
	//fb->DrawPointLight(ppc, pointLightImg, pt);
	pointLightImg->show();
	pointLightImg ->SaveTiff("point_light.tif");
}
/* Visualize point light */
void Scene::VisualizePointLight(V3 L){
	FrameBuffer *pointLightImg = new FrameBuffer(20, 50, 640, 480);
	pointLightImg->Set(0xFF000000);
	pointLightImg->label("Point Light");
	pointLightImg->DrawPointLight(ppc, L);
	pointLightImg->show();
}

/* Render Filled SM1 */
void Scene::SM1(){
	int renderMode = 1;
	V3 L = V3(0, 0, 0);
	fb->Clear(0xFFFFFFFF, 0.0f);
	tmeshes[1]->RenderFilled(ppc, ppc,fb, 0xFF0000FF, L, 0,0, renderMode,0,0,texture);
	fb->redraw();
}

/* Render Filled SM2 */
void Scene::SM2(){
	int renderMode = 2;
	V3 L;
	float ka,es;
	L[0] = atof(pointLightx());
	L[1] = atof(pointLighty());
	L[2] = atof(pointLightz());
	ka = atof(Ka());
	es = atof(Es());
	fb->Clear(0xFFFFFFFF, 0.0f);
	tmeshes[1]->RenderFilled(ppc, ppc,fb, 0xFF0000FF, L, ka, es, renderMode,0,0,texture);
	fb->redraw();
}

/* Render Filled SM3 */
void Scene::SM3(){
	int renderMode = 3;
	V3 L;
	float ka,es;
	L[0] = atof(pointLightx());
	L[1] = atof(pointLighty());
	L[2] = atof(pointLightz());
	ka = atof(Ka());
	es = atof(Es());
	fb->Clear(0xFFFFFFFF, 0.0f);
	tmeshes[1]->RenderFilled(ppc, ppc,fb, 0xFF0000FF, L, ka, es, renderMode,0,0,texture);
	fb->redraw();
}

/* Nearest Neighbor Look Up */
void Scene::Nearest(){
	int lookupMode = 1;
	int tilingMode = 0;
	int renderMode = 4;
	V3 L = V3(0,0,0);
	fb->Clear(0xFFFFFFFF, 0.0f);
	tmeshes[0]->RenderFilled(ppc,ppc, fb, 0xFF0000FF, L, 0, 0, renderMode, tilingMode,lookupMode,texture);
	fb->redraw();
}

void Scene::Bilinear(){
	int lookupMode = 2;
	int tilingMode = 0;
	int renderMode = 4;
	V3 L = V3(0, 0, 0);
	fb->Clear(0xFFFFFFFF, 0.0f);
	tmeshes[0]->RenderFilled(ppc,ppc, fb, 0xFF0000FF, L, 0, 0, renderMode, tilingMode,lookupMode,texture);
	fb->redraw();
}

void Scene::Repeat(){
	int lookupMode = 1;
	int tilingMode = 1;
	int renderMode = 4;
	V3 L = V3(0, 0, 0);
	fb->Clear(0xFFFFFFFF, 0.0f);
	tmeshes[0]->RenderFilled(ppc, ppc,fb, 0xFF0000FF, L, 0, 0, renderMode, tilingMode,lookupMode,texture);
	fb->redraw();
}

void Scene::Mirror(){
	int lookupMode = 1;
	int tilingMode = 2;
	int renderMode = 4;
	V3 L = V3(0, 0, 0);
	fb->Clear(0xFFFFFFFF, 0.0f);
	tmeshes[0]->RenderFilled(ppc,ppc, fb, 0xFF0000FF, L, 0, 0, renderMode, tilingMode,lookupMode,texture);
	fb->redraw();
}

void Scene::Shadow(){

	unsigned int color = 0xFF000000;
	V3 L = V3(0, 0, 0);
	if (gui->pointLightx->value() && gui->pointLighty->value() && gui->pointLightz->value()){
		L[0] = atof(pointLightx());
		L[1] = atof(pointLighty());
		L[2] = atof(pointLightz());
	}

	fb->Clear(0xFF000000, 0);
	shadowMap->Set(0xFFFFFFFF);
	tmeshes[9]->RenderFilled(ppc, 0, fb, color, L, 0, 0, 4, 0, 1, ground);
	
	renderMode = 1;
	
	// create two meshes
	V3 newC = V3(-30.0f, -8.0f, -230.0f);
	tmeshes[7]->Position(newC);
	newC = V3(0.0f, -20.0f, -200.0f);
	tmeshes[8]->Position(newC);
	tmeshes[8]->Scale(.3);


	// render shadow map
	//shadowfb = fb;
	lightView = new PPC(55.0f, 640, 480);
	lightView->C = L;
	
	tmeshes[7]->RenderFilled(lightView, lightView,fb, 0xFFFFFFFF, L, 0, 0, 5, 0, 0, shadowMap);
	tmeshes[8]->RenderFilled(lightView, lightView,fb, 0xFFFFFFFF, L, 0, 0, 5, 0, 0, shadowMap);
	tmeshes[9]->RenderFilled(lightView, lightView, fb, 0xFFFFFFFF, L, 0, 0, 5, 0, 0, shadowMap);
	//shadowMap->show();

	// render scene
	renderMode = 6;
	tmeshes[7]->RenderFilled(ppc, lightView,fb, 0xFF0000FF, L, 0.2, 0, renderMode, tilingMode, lookupMode, shadowMap);
	tmeshes[8]->RenderFilled(ppc, lightView,fb, 0xFF0000FF, L, 0.2, 0, renderMode, tilingMode, lookupMode, shadowMap);
	tmeshes[9]->RenderFilled(ppc, lightView, fb, 0xFF0000FF, L, 0.2, 0, renderMode, tilingMode, lookupMode, shadowMap);

	for (int i = 0; i < 5; i++)
		ZoomIn();

	for (int i = 0; i < 10; i++){
		Left();
	}
	
	int fi = 0;
	for (int i = 0; i < 20; i++) {
		V3 step = V3(-10, 0, 0);
		lightView = new PPC(55.0f, 640, 480);
		L = L + step;
		lightView->C = L;

		tmeshes[7]->RenderFilled(lightView, lightView, fb, 0xFFFFFFFF, L, 0, 0, 5, 0, 0, shadowMap);
		tmeshes[8]->RenderFilled(lightView, lightView, fb, 0xFFFFFFFF, L, 0, 0, 5, 0, 0, shadowMap);
		tmeshes[9]->RenderFilled(lightView, lightView, fb, 0xFFFFFFFF, L, 0, 0, 5, 0, 0, shadowMap);
		//shadowMap->show();

		// render scene
		renderMode = 6;
		tmeshes[7]->RenderFilled(ppc, lightView, fb, 0xFF0000FF, L, 0.2, 0, renderMode, tilingMode, lookupMode, shadowMap);
		tmeshes[8]->RenderFilled(ppc, lightView, fb, 0xFF0000FF, L, 0.2, 0, renderMode, tilingMode, lookupMode, shadowMap);
		tmeshes[9]->RenderFilled(ppc, lightView, fb, 0xFF0000FF, L, 0.2, 0, renderMode, tilingMode, lookupMode, shadowMap);

		fb->redraw();
		Fl::check();

		//string fname = "movies/" + to_string(fi) + ".tif";
		//fb->SaveTiff(fname.c_str());
		fi++;
	}

	for (int i = 0; i < 20; i++) {
		V3 step = V3(10, 0, 0);
		lightView = new PPC(55.0f, 640, 480);
		L = L + step;
		lightView->C = L;

		tmeshes[7]->RenderFilled(lightView, lightView, fb, 0xFFFFFFFF, L, 0, 0, 5, 0, 0, shadowMap);
		tmeshes[8]->RenderFilled(lightView, lightView, fb, 0xFFFFFFFF, L, 0, 0, 5, 0, 0, shadowMap);
		tmeshes[9]->RenderFilled(lightView, lightView, fb, 0xFFFFFFFF, L, 0, 0, 5, 0, 0, shadowMap);
		shadowMap->show();

		// render scene
		renderMode = 6;
		tmeshes[7]->RenderFilled(ppc, lightView, fb, 0xFF0000FF, L, 0.2, 0, renderMode, tilingMode, lookupMode, shadowMap);
		tmeshes[8]->RenderFilled(ppc, lightView, fb, 0xFF0000FF, L, 0.2, 0, renderMode, tilingMode, lookupMode, shadowMap);
		tmeshes[9]->RenderFilled(ppc, lightView, fb, 0xFF0000FF, L, 0.2, 0, renderMode, tilingMode, lookupMode, shadowMap);

		fb->redraw();
		Fl::check();

		//string fname = "movies/" + to_string(fi) + ".tif";
		//fb->SaveTiff(fname.c_str());
		fi++;
	}

	

	for (int i = 0; i < 20; i++) {
		V3 step = V3(0, 10, 0);
		lightView = new PPC(55.0f, 640, 480);
		L = L + step;
		lightView->C = L;

		tmeshes[7]->RenderFilled(lightView, lightView, fb, 0xFFFFFFFF, L, 0, 0, 5, 0, 0, shadowMap);
		tmeshes[8]->RenderFilled(lightView, lightView, fb, 0xFFFFFFFF, L, 0, 0, 5, 0, 0, shadowMap);
		tmeshes[9]->RenderFilled(lightView, lightView, fb, 0xFFFFFFFF, L, 0, 0, 5, 0, 0, shadowMap);
		//shadowMap->show();

		// render scene
		renderMode = 6;
		tmeshes[7]->RenderFilled(ppc, lightView, fb, 0xFF0000FF, L, 0.2, 0, renderMode, tilingMode, lookupMode, shadowMap);
		tmeshes[8]->RenderFilled(ppc, lightView, fb, 0xFF0000FF, L, 0.2, 0, renderMode, tilingMode, lookupMode, shadowMap);
		tmeshes[9]->RenderFilled(ppc, lightView, fb, 0xFF0000FF, L, 0.2, 0, renderMode, tilingMode, lookupMode, shadowMap);

		fb->redraw();
		Fl::check();

		//string fname = "movies/" + to_string(fi) + ".tif";
		//fb->SaveTiff(fname.c_str());
		fi++;
	}
	
	for (int i = 0; i < 20; i++) {
		V3 step = V3(0, -10, 0);
		lightView = new PPC(55.0f, 640, 480);
		L = L + step;
		lightView->C = L;

		tmeshes[7]->RenderFilled(lightView, lightView, fb, 0xFFFFFFFF, L, 0, 0, 5, 0, 0, shadowMap);
		tmeshes[8]->RenderFilled(lightView, lightView, fb, 0xFFFFFFFF, L, 0, 0, 5, 0, 0, shadowMap);
		tmeshes[9]->RenderFilled(lightView, lightView, fb, 0xFFFFFFFF, L, 0, 0, 5, 0, 0, shadowMap);
		//shadowMap->show();

		// render scene
		renderMode = 6;
		tmeshes[7]->RenderFilled(ppc, lightView, fb, 0xFF0000FF, L, 0.2, 0, renderMode, tilingMode, lookupMode, shadowMap);
		tmeshes[8]->RenderFilled(ppc, lightView, fb, 0xFF0000FF, L, 0.2, 0, renderMode, tilingMode, lookupMode, shadowMap);
		tmeshes[9]->RenderFilled(ppc, lightView, fb, 0xFF0000FF, L, 0.2, 0, renderMode, tilingMode, lookupMode, shadowMap);

		fb->redraw();
		Fl::check();

		//string fname = "movies/" + to_string(fi) + ".tif";
		//fb->SaveTiff(fname.c_str());
		fi++;
	}
	/*

	int fi = 0;
	for (int i = 0; i < 5; i++){
		ZoomIn();
		fb->redraw();
		Fl::check();
		string fname = "movies/" + to_string(fi) + ".tif";
		fb->SaveTiff(fname.c_str());
		fi++;
	}
	
	for (int i = 0; i < 20; i++){
		Left();
		fb->redraw();
		Fl::check();
		string fname = "movies/" + to_string(fi) + ".tif";
		fb->SaveTiff(fname.c_str());
		fi++;
	}
	for (int i = 0; i < 40; i++){
		Right();
		fb->redraw();
		Fl::check();
		string fname = "movies/" + to_string(fi) + ".tif";
		fb->SaveTiff(fname.c_str());
		fi++;
	}
	for (int i = 0; i < 20; i++){
		Left();
		fb->redraw();
		Fl::check();
		string fname = "movies/" + to_string(fi) + ".tif";
		fb->SaveTiff(fname.c_str());
		fi++;
	} */
	
	fb->redraw();
}

void Scene::EMSW(){
	unsigned int color;	
	V3 lightPoint(tmeshes[1]->GetCenter() + V3(0.0f, 0.0f, 100.0f));
	float ka = .0f;

	/*for (int i = 0; i < 6; i++)
		cerr << "i: " << i << " w: " << em->face[i]->w << " h: " << em->face[i]->h << endl;*/

	// render the background
	/*
	for (int v = 0; v < fb->h; v++) {
		for (int u = 0; u < fb->w; u++) {
			//fb->Set(0xFFFFFFFF);
			//fb->SetZ(u, v, FLT_MAX);
			V3 ray = (ppc->c + ppc->a*((float)u + .5f) + ppc->b*((float)v + .5f)).UnitVector();
			color = em->GetColor(ray, ppc->C);
			fb->Set(u, v, color);
		}
	}
	*/
	
	cerr << "Start initialization..." << endl;
	// compute projected vertices
	V3 **pverts = (V3 **)malloc(sizeof(V3*)*tmeshesN);
	bool **proj_flag = (bool **)malloc(sizeof(bool*)*tmeshesN);
	M33 **nABC = (M33**)malloc(sizeof(M33*)*tmeshesN);

	for (int tmi = 0; tmi < tmeshesN; tmi++){
		if (!tmeshes[tmi]->enabled)
			continue;

		pverts[tmi] = new V3[tmeshes[tmi]->vertsN];
		proj_flag[tmi] = new bool[tmeshes[tmi]->vertsN];
		nABC[tmi] = new M33[tmeshes[tmi]->trisN];

		for (int vi = 0; vi < tmeshes[tmi]->vertsN; vi++) {
			proj_flag[tmi][vi] = ppc->Project(tmeshes[tmi]->verts[vi], pverts[tmi][vi]);
		}

		for (int tri = 0; tri < tmeshes[tmi]->trisN; tri++) {

			int vinds[3];
			vinds[0] = tmeshes[tmi]->tris[tri * 3 + 0];
			vinds[1] = tmeshes[tmi]->tris[tri * 3 + 1];
			vinds[2] = tmeshes[tmi]->tris[tri * 3 + 2];

			if (!proj_flag[tmi][vinds[0]] || !proj_flag[tmi][vinds[1]] || !proj_flag[tmi][vinds[2]])
				continue;

			V3 n[3];
			n[0] = tmeshes[tmi]->normals[tmeshes[tmi]->tris[3 * tri + 0]];
			n[1] = tmeshes[tmi]->normals[tmeshes[tmi]->tris[3 * tri + 1]];
			n[2] = tmeshes[tmi]->normals[tmeshes[tmi]->tris[3 * tri + 2]];

			nABC[tmi][tri] = tmeshes[tmi]->GetNABC(pverts[tmi][vinds[0]], pverts[tmi][vinds[1]], pverts[tmi][vinds[2]], n[0], n[1], n[2]);
		}
	}

	cerr << "Start rendering" << endl;
		

	// render the reflective teapot
	for (int v = 0; v < fb->h; v++) {

		for (int u = 0; u < fb->w; u++) {
			//fb->Set(u, v, 0xFFAAAAAA);
			// render the background
			V3 ray = (ppc->c + ppc->a*((float)u + .5f) + ppc->b*((float)v + .5f)).UnitVector();
			color = em->GetColor(ray, ppc->C);
			fb->Set(u, v, color);
			
			// set z buffer
			fb->SetZ(u, v, FLT_MAX);

			V3 pv = V3(u + 0.5, v + 0.5, 1.0);  // current pixel is pv
			V3 ev = (pv-ppc->C).Normalized();	// ev is eye vector



			for (int tmi = 0; tmi < tmeshesN; tmi++) {
				if (!tmeshes[tmi]->enabled)
					continue;

				

				for (int tri = 0; tri < tmeshes[tmi]->trisN; tri++) {
					V3 Vs[3];
					Vs[0] = tmeshes[tmi]->verts[tmeshes[tmi]->tris[3 * tri + 0]];
					Vs[1] = tmeshes[tmi]->verts[tmeshes[tmi]->tris[3 * tri + 1]];
					Vs[2] = tmeshes[tmi]->verts[tmeshes[tmi]->tris[3 * tri + 2]];

					int vinds[3];
					vinds[0] = tmeshes[tmi]->tris[tri * 3 + 0];
					vinds[1] = tmeshes[tmi]->tris[tri * 3 + 1];
					vinds[2] = tmeshes[tmi]->tris[tri * 3 + 2];

					if (!proj_flag[tmi][vinds[0]] || !proj_flag[tmi][vinds[1]] || !proj_flag[tmi][vinds[2]])
						continue;

					/*V3 zABC = tmeshes[tmi]->GetZABC(pverts[vinds[0]], pverts[vinds[1]], pverts[vinds[2]]);
					float currz = zABC*pv;
					if (fb->IsOutsideFrame((int)u, (int)v))
						continue;
					if (fb->IsFarther((int)u, (int)v, currz))
						continue;
					fb->SetZ((int)u, (int)v, currz);*/
									
					V3 bgt = ppc->C.IntersectRayWithTriangleWithThisOrigin(
						ray, Vs);
					if (bgt[2] < 0.0f || bgt[0] < 0.0f || bgt[1] < 0.0f ||
						bgt[0] + bgt[1] > 1.0f)
						continue;
					if (fb->zb[(fb->h - 1 - v)*fb->w + u] < bgt[2])
						continue;
					fb->SetZ(u, v, bgt[2]);
					float currz = bgt[2];
					float alpha = 1.0f - bgt[0] - bgt[1];
					float beta = bgt[0];
					float gamma = bgt[1];

			
					/*V3 x, y, normal;
					x = tmeshes[tmi]->verts[tmeshes[tmi]->tris[3 * tri + 1]] - tmeshes[tmi]->verts[tmeshes[tmi]->tris[3 * tri + 0]];
					y = tmeshes[tmi]->verts[tmeshes[tmi]->tris[3 * tri + 2]] - tmeshes[tmi]->verts[tmeshes[tmi]->tris[3 * tri + 0]];
					x.Normalized();
					y.Normalized();
					normal = x^y;
					normal.Normalized();*/

					// compute normal for triangle
					//M33 nABC = tmeshes[tmi]->GetNABC(Vs[0], Vs[1], Vs[2], n[0], n[1], n[2]);
					V3 normal = tmeshes[tmi]->SSIN(pv, nABC[tmi][tri][0], nABC[tmi][tri][1], nABC[tmi][tri][2]);
					

					// compute reflected ray and get color
					V3 reflectedRay = ev - normal*(normal*ev)*2.0f;
					reflectedRay.Normalized();
					//reflectedRay = (reflectedRay*(-1.0f)).Normalized();
					//V3 reflectedRay = ev - (normal*2.0f*(normal*ev));
					unsigned int color = 0;
					color = em->GetColor(reflectedRay, ppc->C);


					
					//V3 color = Cs[0] * alpha + Cs[1] * beta + Cs[2] * gamma;
					fb->Set(u, v, color);
					//cerr << "color:" << color << endl;
				}
				
			}
		}
		fb->redraw();
		Fl::check();
	}
	

	/*for (int tmi = 0; tmi < tmeshesN; tmi++){
		delete[] pverts[tmi];
		delete[] proj_flag[tmi];
		delete[] nABC[tmi];
	}*/

	
	delete pverts;
	delete proj_flag;
	delete nABC;

	/*V3 center = V3(0.0f, 0.0f, 0.0f);
	tmeshes[tmi]->Position(center);
	tmeshes[tmi]->RenderFilled(ppc, ppc, fb, color, lightPoint, ka, 0.5, renderMode, tilingMode, lookupMode, texture);
	fb->redraw();*/
	//Fl::check();
}


void Scene::Panorama(){
	
	FrameBuffer  *panorama, *left, *right;
	left->LoadTiff("left.tif", left);
	right->LoadTiff("right.tif", right);
	panorama = left->Stitch(right);
	//panorama->show();
	//panorama->SaveTiff("coarsePanorama.tif");
	//panorama->LoadTiff("panorama.tif", panorama);

	
	int w_mid = panorama->w / 2;
	int h_mid = panorama->h / 2;

	unsigned int color;

	list<PPC> camera_path = ppc->LoadCameraPathFromTxt("GUICameraPath.txt");
	int fi = 0;
	for (list<PPC>::iterator it = camera_path.begin(); it != camera_path.end(); it++){
		fb->Clear(0xFFFFFFFF, 0.0f);
		PPC *point = &*it;

		float delta = point->C[0];
		for (int v = 0; v < fb->h; v++){
			for (int u = 0; u < fb->w; u++){

				color = panorama->Get(w_mid - u + delta, v);
				fb->Set(u, v, color);
			}
		}
		fb->redraw();
		
		Fl::check();
		string fname = "A10movie/" + to_string(fi) + ".tif";
		fb->SaveTiff(fname.c_str());
		fi++;
	}
	
}