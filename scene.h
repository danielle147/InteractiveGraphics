#pragma once

#include "gui.h"
#include "framebuffer.h"
#include "ppc.h"
#include "tmesh.h"
#include "envmap.h"

#include "CGInterface.h"

class Scene {
public:
  //gpu programming
  CGInterface *cgi;
  ShaderOneInterface *soi;
  
  GUI *gui;
  FrameBuffer *fb,*hwfb,*texture,*texture1,*texture2,*shadowMap,*ground;
  PPC *ppc, *lightView;
  TMesh **tmeshes;
  int tmeshesN;
  unsigned int *texHW;
  EnvMap *em;

  bool hwInit; // false until graphics hardware rendering is initialized
  bool isEnvMap;
  bool isPanorama;
 
  Scene();
  void DBG();
  void SetTexture();
  void RenderHW();
  void Render();
  void FlagB();
  void LoadFile();
  const char* ReadFile();
  const char* SaveTo();
  void SaveFile();
  const char* BrightPercent();
  void AdjustBrightness();
  void DrawName();
  void EdgeDetect();
  void Visualize();
  void Up();
  void Down();
  void Left();
  void Right();
  void Front();
  void Back();
  void PanRight();
  void PanLeft();
  void TiltUp();
  void TiltDown();
  void RollLeft();
  void RollRight();
  void ZoomIn();
  void ZoomOut();
  void CreatePPCPath();
  void Play();
  void PlayHW();
  void SM2Play();
  void SM3Play();
  void SM2SM3();
  void Display();
  void DrawPointLight();
  void VisualizePointLight(V3 L);
  const char* pointLightx();
  const char* pointLighty();
  const char* pointLightz();
  const char* Ka();
  const char* Es();
  void SM1();
  void SM2();
  void SM3();
  void Nearest();
  void Bilinear();
  void Repeat();
  void Mirror();
  void Shadow();
  void EMSW();
  void Panorama();

  void RayTrace();

  // animation parameters
  float animationFraction, animationRadius;
  V3 animationCenter;
  V3 light;
};

extern Scene *scene;
