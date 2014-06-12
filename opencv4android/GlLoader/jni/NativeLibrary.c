#include <jni.h>
#include <EGL/egl.h> // requires ndk r5 or newer
#include <GLES/gl.h>

#include "NativeLibrary.h"
//~ #include "cube.h"
//~ #include "rossini.h"
//~ #include "rossini2.h"
#include "rossini3.h"
#include "rossiniColors.h"

void InitializeOpenGL()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
}

void resizeViewport(int newWidth, int newHeight)
{
	glViewport(0, 0, newWidth, newHeight);
}

//~ static GLfloat COLORS[] = {
        //~ 1.0f, 0.0f, 0.0f, 1.0f, // red
        //~ 1.0f, 0.0f, 0.0f, 1.0f, // red
        //~ 1.0f, 0.0f, 0.0f, 1.0f, // red
        //~ 
        //~ 1.0f, 0.0f, 0.0f, 1.0f, // red
        //~ 1.0f, 0.0f, 0.0f, 1.0f, // red
        //~ 1.0f, 0.0f, 0.0f, 1.0f, // red
        //~ 
        //~ 1.0f, 0.0f, 0.0f, 1.0f, // red
        //~ 1.0f, 0.0f, 0.0f, 1.0f, // red
        //~ 1.0f, 0.0f, 0.0f, 1.0f, // red
        //~ 
        //~ 1.0f, 0.0f, 0.0f, 1.0f, // red
        //~ 1.0f, 0.0f, 0.0f, 1.0f, // red
        //~ 1.0f, 0.0f, 0.0f, 1.0f, // red
        //~ 
        //~ 1.0f, 0.0f, 0.0f, 1.0f, // red
        //~ 1.0f, 0.0f, 0.0f, 1.0f, // red
        //~ 1.0f, 0.0f, 0.0f, 1.0f, // red
        //~ 
        //~ 1.0f, 0.0f, 0.0f, 1.0f, // red
        //~ 1.0f, 0.0f, 0.0f, 1.0f, // red
        //~ 1.0f, 0.0f, 0.0f, 1.0f, // red
        //~ 
        //~ 1.0f, 0.0f, 0.0f, 1.0f, // red
        //~ 1.0f, 0.0f, 0.0f, 1.0f, // red
        //~ 1.0f, 0.0f, 0.0f, 1.0f, // red
        //~ 
        //~ 1.0f, 0.0f, 0.0f, 1.0f, // red
        //~ 1.0f, 0.0f, 0.0f, 1.0f, // red
        //~ 1.0f, 0.0f, 0.0f, 1.0f, // red
        //~ 
        //~ 1.0f, 0.0f, 0.0f, 1.0f, // red
        //~ 1.0f, 0.0f, 0.0f, 1.0f, // red
        //~ 1.0f, 0.0f, 0.0f, 1.0f, // red
        //~ 
        //~ 1.0f, 0.0f, 0.0f, 1.0f, // red
        //~ 1.0f, 0.0f, 0.0f, 1.0f, // red
        //~ 1.0f, 0.0f, 0.0f, 1.0f, // red
        //~ 
        //~ 1.0f, 0.0f, 0.0f, 1.0f, // red
        //~ 1.0f, 0.0f, 0.0f, 1.0f, // red
        //~ 1.0f, 0.0f, 0.0f, 1.0f, // red
        //~ 
        //~ 1.0f, 0.0f, 0.0f, 1.0f, // red
        //~ 1.0f, 0.0f, 0.0f, 1.0f, // red
        //~ 1.0f, 0.0f, 0.0f, 1.0f // red
//~ };

//~ void renderFrame(float projection[], float pose[])
//~ {
	//~ float scale = 100.0f;
	//~ glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//~ glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // transparent
//~ 
        //~ glMatrixMode(GL_PROJECTION);
        //~ glLoadMatrixf(projection);
        //~ 
        //~ glMatrixMode(GL_MODELVIEW);
        //~ glLoadMatrixf(pose);
        //~ glTranslatef(0.0f, 0.0f, 0.0f);
        //~ //gl.glTranslatef(0f,0f,0f);
        //~ glScalef(scale, scale, scale);
//~ 
    //~ glEnableClientState(GL_VERTEX_ARRAY);
    //~ glEnableClientState(GL_COLOR_ARRAY);
//~ 
    //~ glVertexPointer(3, GL_FLOAT, 0, cubeVerts);
	//~ glNormalPointer(GL_FLOAT, 0, cubeNormals);
    //~ glColorPointer(4, GL_FLOAT, 0, COLORS);
//~ 
    //~ glDrawArrays(GL_TRIANGLES,0,cubeNumVerts);
//~ }

void renderFrame(float projection[], float pose[])
{
	float scale = 500.0f;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // transparent
	//glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);

        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(projection);
        
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(pose);
        glTranslatef(0.0f, 0.0f, 0.0f);
        //gl.glTranslatef(0f,0f,0f);
        glScalef(scale, scale, scale);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

glVertexPointer(3, GL_FLOAT, 0, rossiniVerts);
//~ glNormalPointer(GL_FLOAT, 0, rossiniNormals);
//~ glTexCoordPointer(2, GL_FLOAT, 0, rossiniTexCoords);

    glColorPointer(4, GL_FLOAT, 0, COLORS);

    glDrawArrays(GL_TRIANGLES,0,rossiniNumVerts);
}

void renderVideoFrame(float projection[], float pose[])
{
	float scale = 500.0f;
	//~ float scale = 100.0f;
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // transparent
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	//glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);

        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(projection);
        
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(pose);
        glTranslatef(0.0f, 0.0f, 0.0f);
        //gl.glTranslatef(0f,0f,0f);
        glScalef(scale, scale, scale);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

glVertexPointer(3, GL_FLOAT, 0, rossiniVerts);
//~ glNormalPointer(GL_FLOAT, 0, rossiniNormals);
//~ glTexCoordPointer(2, GL_FLOAT, 0, rossiniTexCoords);

    glColorPointer(4, GL_FLOAT, 0, COLORS);

    glDrawArrays(GL_TRIANGLES,0,rossiniNumVerts); 
}
