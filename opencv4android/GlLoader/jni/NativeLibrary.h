#ifndef NATIVE_LIBRARY_H
#define NATIVE_LIBRARY_H

void InitializeOpenGL();
void resizeViewport(int newWidth, int newHeight);
static float VERTICES[];
static float COLORS[];
void renderFrame(float projection[], float pose[]);
void renderVideoFrame(float projection[], float pose[]);

float angle = 0;

// SDL timing
long oldTime = 0;
long newTime = 0;

// Functions
void draw();
#endif
