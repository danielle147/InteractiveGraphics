#include "framebuffer.h"
#include "v3.h"
#include <iostream>
#include "scene.h"
#include "tiffio.h"
#include <Windows.h>
#include "ppc.h"

FrameBuffer::FrameBuffer(int u0, int v0, 
  int _w, int _h) : Fl_Gl_Window(u0, v0, _w, _h, 0) {

  w = _w;
  h = _h;
  pix = new unsigned int[w*h];
  zb = new float[w*h];
  isHW = false;

}


void FrameBuffer::draw() {

	if (!isHW) {
		glDrawPixels(w, h, GL_RGBA, GL_UNSIGNED_BYTE, pix);
	}
	else {
		scene->RenderHW();
	}
}


void FrameBuffer::Set(unsigned int bgr) {

  for (int i = 0; i < w*h; i++) {
    pix[i] = bgr;
  }

}

void FrameBuffer::Set(int u, int v, unsigned int color) {

	pix[(h-1-v)*w+u] = color;

}

unsigned int FrameBuffer::Get(int u, int v) {

  return pix[(h-1-v)*w+u];

}


void FrameBuffer::SetSafe(int u, int v, unsigned int color) {

  if (u < 0 || u > w-1 || v < 0 || v > h-1)
    return;

  Set(u, v, color);

}


void FrameBuffer::SetChecker(int csize, unsigned int color0, 
  unsigned int color1) {


  for (int v = 0; v < h; v++) {
    int cv = v / csize;
    for (int u = 0; u < w; u++) {
      int cu = u / csize;
      if ( (cu+cv) % 2)
        Set(u, v, color1);
      else
        Set(u, v, color0);
    }
  }

}


void FrameBuffer::DrawPoint(int u, int v, int psize, unsigned int color) {

  for (int i = v - psize/2; i <= v + psize/2; i++) {
    for (int j = u - psize/2; j <= u + psize/2; j++) {
      SetSafe(j, i, color);
    }
  }

}

/* Line resterization */
void FrameBuffer::DrawSegment(V3 pp0, V3 pp1, V3 c0, V3 c1) {

	float u0f = pp0[0];
	float u1f = pp1[0];
	float v0f = pp0[1];
	float v1f = pp1[1];

	float du = fabsf(u1f - u0f);
	float dv = fabsf(v1f - v0f);
	int stepsN = (du < dv) ? 1 + (int)dv : 1 + (int)du;

	V3 startingPoint(pp0);
	V3 endingPoint(pp1);
	V3 currentPoint;
	int segsN;
	if (stepsN == 1)
		segsN = 1;
	else
		segsN = stepsN - 1;
	V3 segmentStep = (endingPoint - startingPoint) / (float)segsN;
	V3 colorStep = (c1 - c0) / (float)segsN;
	int i;
	V3 cc;
	for (i = 0,
		currentPoint = startingPoint,
		cc = c0;
	i < stepsN;
	i++,
		currentPoint = currentPoint + segmentStep,
		cc = cc + colorStep) {
		int u = (int)currentPoint[0];
		int v = (int)currentPoint[1];
		if (IsOutsideFrame(u, v))
			continue;
		if (IsFarther(u, v, currentPoint[2]))
			continue;
		SetZ(u, v, currentPoint[2]);
		Set(u, v, cc.GetColor());
	}

}

void FrameBuffer::Convolve33(M33 kernel, FrameBuffer *&fb1) {

  fb1 = new FrameBuffer(30 + w, 30, w, h);

  for (int v = 1; v < h-1; v++) {
    for (int u = 1; u < w-1; u++) {
      V3 newColor(0.0f, 0.0f, 0.0f);
      for (int vi = -1; vi <= 1; vi++) {
        for (int ui = -1; ui <= 1; ui++) {
          V3 currColor;
          currColor.SetFromColor(Get(u+ui, v+vi));
          newColor = newColor + currColor * kernel[vi+1][ui+1];
        }
      }
      unsigned int newc = newColor.GetColor();
      fb1->Set(u, v, newc);
    }
  }

}
/* Load tiff image file */
unsigned int FrameBuffer::LoadTiff(const char* filename, FrameBuffer *&fb){
	if (filename == ""){
		cerr << "FILE NAME ERROR!" << endl;
		return -1;
	}
	else{
		TIFF* tif = TIFFOpen(filename, "r");
		UINT32 w, h;

		TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
		TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);

		fb = new FrameBuffer(w, h, w, h);

		unsigned int check = TIFFReadRGBAImage(tif, w, h, fb->pix, 0);

		if (!check)
			cerr << "READ IMAGE ERROR!" << endl;

		TIFFClose(tif);
		return check;
	}
}

/* Save tiff image file */
void FrameBuffer::SaveTiff(const char* fileSave){
	
	TIFF* tif = TIFFOpen(fileSave, "w");

	int sampleperpix = 4; 
	char *image = (char*) pix;

	TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, w);
	TIFFSetField(tif, TIFFTAG_IMAGELENGTH, h);
	TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, sampleperpix);
	TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
	TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
	
	tsize_t linebytes = sampleperpix*w;
	
	unsigned char *buf = NULL;
	if (TIFFScanlineSize(tif) < linebytes)
		buf = (unsigned char*)_TIFFmalloc(linebytes);
	else
		buf = (unsigned char*)_TIFFmalloc(TIFFScanlineSize(tif));
	TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tif,w*sampleperpix));

	for (int row = 0; row < h; row++){
		memcpy(buf, &image[(h - row - 1)*linebytes], linebytes);
		if (TIFFWriteScanline(tif, buf, row, 0) < 0)
			break;
	}

	(void)TIFFClose(tif);
	if (buf)
		_TIFFfree(buf);
}

/* Circle rasterization */
void FrameBuffer::DrawCircle(float cx, float cy, float r, float size, unsigned int color){
	float x, y, r2;
	r2 = r * r;
	DrawPoint(cx, cy+r, size, color);
	DrawPoint(cx, cy-r, size, color);
	DrawPoint(cx+r, cy, size, color);
	DrawPoint(cx-r, cy, size, color);

	y = r;
	x = 1.0f;
	y = (float)(sqrt(r2 - 1) + 0.5);

	while (x < y){
		DrawPoint(cx+x, cy+y, size, color);
		DrawPoint(cx+x, cy-y, size, color);
		DrawPoint(cx-x, cy+y, size, color);
		DrawPoint(cx-x, cy-y, size, color);
		DrawPoint(cx+y, cy+x, size, color);
		DrawPoint(cx+y, cy-x, size, color);
		DrawPoint(cx-y, cy+x, size, color);
		DrawPoint(cx-y, cy-x, size, color);
		x += 1;
		y = (float)(sqrt(r2 - x*x) + 0.5);
	}
	if (x == y){
		DrawPoint(cx+x, cy+y, size, color);
		DrawPoint(cx+x, cy-y, size, color);
		DrawPoint(cx-x, cy+y, size, color);
		DrawPoint(cx-x, cy-y, size, color);
	}
}

/* Adjust Brightness */
void FrameBuffer::AdjustBrightness(int percent){
	float increaseFactor = (float)((100.0f + percent) / 100.0f);
	unsigned int color;
	V3 currColor, newColor;
	for (int i = 0; i < w-1; i++){
		for (int j = 0; j < h-1; j++){
			currColor.SetFromColor(Get(i, j));
			newColor = currColor*increaseFactor;
			color = newColor.GetColor();
			Set(i, j, color);
		}
	}
}

/* Edge detection */
void FrameBuffer::EdgeDetect(M33 kernel, FrameBuffer *&fb1){
	V3 currColor, newColor;
	unsigned int accumulator;
	fb1 = new FrameBuffer(30 + w, 30, w, h);

	for (int v = 1; v < h - 1; v++){
		for (int u = 1; u < w - 1; u++){
			newColor = V3(0.0f, 0.0f, 0.0f);
			for (int vi = -1; vi <= 1; vi++){
				for (int ui = -1; ui <= 1; ui++){
					currColor.SetFromColor(Get(u + ui, v + vi));
					newColor = newColor + currColor*kernel[vi+1][ui+1];
				}
			}
			accumulator = newColor.GetColorGrey();
			fb1->Set(u,v,accumulator);
		}
	}
}

void FrameBuffer::Draw3DPoint(V3 pt, PPC *ppc, int psize, V3 color) {

	V3 ppt;
	if (!ppc->Project(pt, ppt))
		return;
	DrawPoint((int)ppt[0], (int)ppt[1], psize, color.GetColor());
}


void FrameBuffer::Draw3DSegment(V3 p0, V3 p1, PPC *ppc, V3 c0, V3 c1) {

	V3 pp0, pp1;
	if (!ppc->Project(p0, pp0))
		return;
	if (!ppc->Project(p1, pp1))
		return;
	DrawSegment(pp0, pp1, c0, c1);

}

bool FrameBuffer::IsOutsideFrame(int u, int v) {

	bool ret;
	ret = u < 0 || u > w - 1 || v < 0 || v > h - 1;
	return ret;

}

void FrameBuffer::Clear(unsigned int bgr, float z0) {

	Set(bgr);
	for (int i = 0; i < w*h; i++) {
		zb[i] = z0;
	}

}

void FrameBuffer::SetZ(int u, int v, float currz) {

	zb[(h - 1 - v)*w + u] = currz;

}

float FrameBuffer::GetZ(int u, int v){
	return zb[(h - 1 - v)*w + u];
}

bool FrameBuffer::IsFarther(int u, int v, float currz) {

	if (zb[(h - 1 - v)*w + u] >= currz)
		return true;

	return false;

}

void FrameBuffer::DrawPointLight(PPC *ppc, V3 pt){
	//fb1 = new FrameBuffer(30 + w, 30, w, h);
	V3 color;
	color.SetFromColor(0xff00ffff);
	cerr << pt[0]<< " "<<pt[1]<<" "<<pt[2] << endl;
	V3 pp;
	if (!ppc->Project(pt, pp)){
		cerr << "invalide project for point light" << endl;
	}
	Draw3DPoint(pt, ppc, 60, color);
}

/* Panorama Stitching */
void FrameBuffer::Panorama(FrameBuffer *left, FrameBuffer *right, FrameBuffer *&panorama){
	int w = 640;
	int h = 480;
	float *sum_diff = (float*)malloc((int)w);

	int w_count = 1;
	int sum = 0;
	
	while (w_count <= w){
		for (int u = 0; u < w_count; u++){
			for (int v = 0; v < h; v++){
				int r = right->Get(u, v);
				int l = left->Get(w - (w_count - u), v);
				int diff_sq = pow(r - l, 2);
				sum += diff_sq;
			}// end v loop
		}// end u loop
		sum = sum / (w_count*h);
		sum_diff[w_count-1] = sum;
		w_count++;
	}// end while

	//debug

	cerr << "sum: "<<sum << endl;

}

/* Stitching algorithm */
FrameBuffer* FrameBuffer::Stitch(FrameBuffer *right){
	int w = 640;
	int h = 480;
	double *sum_diff = (double*)malloc(w*sizeof(double));

	int w_count = 1;
	double sum = 0;

	unsigned int l, r, difference,diff_sq;

	
	// Compute for the stitching point
	while (w_count <= w){
		sum = 0;
		for (int u = 0; u < w_count; u++){
			for (int v = 0; v < h; v++){
				r = right->Get(u, v);
				l = this->Get(w - (w_count - u), v);
				difference = r - l;
				diff_sq = pow(difference, 2);
				sum += diff_sq;
			}// end v loop
		}// end u loop
		sum = (sum*1.0f) / (w_count*h);
		sum_diff[w_count - 1] = sum;
		w_count++;
	
	}// end while

	double min = 0;
	int min_index = 0;
	for (int i = 0; i < w; i++){
		if (min == 0)
			min = sum_diff[i];
		if (sum_diff[i] < min){
			min = sum_diff[i];
			min_index = i;
			//cerr << "min: " <<i<<" "<< min << endl;
		}
	}

	// Start stitching
	int panoramaWidth = w + (w - 1 - min_index) + w - (w - 1 - min_index);
	FrameBuffer *panorama = new FrameBuffer(panoramaWidth, h, panoramaWidth, h);
	unsigned int color;
	for (int v = 0; v < h; v++){
		for (int u = 0; u < w; u++){
			color = this->Get(u, v);
			panorama->Set(u, v, color);
		}
	}
	for (int u = w; u < panoramaWidth; u++){
		for (int v = 0; v < h; v++){
			color = right->Get(u, v);
			panorama->Set(u, v, color);
		}
	}

	//debug
	//cerr << "min_index: " << min_index << endl;
	//cerr << "sum: " << sum << endl;
	//cerr << "l: " << l << endl;
	//cerr << "r: " << r << endl;
	//cerr << "difference: " << difference << endl;
	//cerr << "diff_sqr: " << diff_sq << endl;
	/*for (int i = 0; i < w; i++){
		cerr << "sum_diff: " << i <<" "<< sum_diff[i] << endl;
	}*/
	
	return panorama;
}
