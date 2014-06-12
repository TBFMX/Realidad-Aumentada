//
// Copyright 2011 Tero Saarni
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <android/native_window.h> // requires ndk r5 or newer
#include <EGL/egl.h> // requires ndk r5 or newer
#include <GLES/gl.h>
#include <EGL/egl.h> 

#include "logger.h"
#include "renderer.h"
//~ #include "rossini3.h"
#include "rossini2.h"
//~ #include "rossini.h"
#include "rossiniColors.h"
#define LOG_TAG "EglSample"
 
Renderer::Renderer()
    : _msg(MSG_NONE), _display(0), _surface(0), _context(0), _angle(0)
{
    LOG_INFO("Renderer instance created");
    pthread_mutex_init(&_mutex, 0);    
    return;
}

Renderer::~Renderer()
{
    LOG_INFO("Renderer instance destroyed");
    pthread_mutex_destroy(&_mutex);
    return;
}

void Renderer::start()
{
    LOG_INFO("Creating renderer thread");
    pthread_create(&_threadId, 0, threadStartCallback, this);
    return;
}

void Renderer::stop()
{
    LOG_INFO("Stopping renderer thread");

    // send message to render thread to stop rendering
    pthread_mutex_lock(&_mutex);
    _msg = MSG_RENDER_LOOP_EXIT;
    pthread_mutex_unlock(&_mutex);    

    pthread_join(_threadId, 0);
    LOG_INFO("Renderer thread stopped");

    return;
}

void Renderer::setWindow(ANativeWindow *window)
{
    // notify render thread that window has changed
    pthread_mutex_lock(&_mutex);
    _msg = MSG_WINDOW_SET;
    _window = window;
    pthread_mutex_unlock(&_mutex);

    return;
}



void Renderer::renderLoop()
{
    bool renderingEnabled = true;
    
    LOG_INFO("renderLoop()");

    while (renderingEnabled) {

        pthread_mutex_lock(&_mutex);

        // process incoming messages
        switch (_msg) {

            case MSG_WINDOW_SET:
                initialize();
                break;

            case MSG_RENDER_LOOP_EXIT:
                renderingEnabled = false;
                destroy();
                break;

            default:
                break;
        }
        _msg = MSG_NONE;
        
        if (_display) {
            drawFrame();
            if (!eglSwapBuffers(_display, _surface)) {
                LOG_ERROR("eglSwapBuffers() returned error %d", eglGetError());
            }
        }
        
        pthread_mutex_unlock(&_mutex);
    }
    
    LOG_INFO("Render loop exits");
    
    return;
}

bool Renderer::initialize()
{
    const EGLint attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_NONE
    };
    EGLDisplay display;
    EGLConfig config;    
    EGLint numConfigs;
    EGLint format;
    EGLSurface surface;
    EGLContext context;
    EGLint width;
    EGLint height;
    GLfloat ratio;
    
    LOG_INFO("Initializing context");
    
    if ((display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY) {
        LOG_ERROR("eglGetDisplay() returned error %d", eglGetError());
        return false;
    }
    if (!eglInitialize(display, 0, 0)) {
        LOG_ERROR("eglInitialize() returned error %d", eglGetError());
        return false;
    }

    if (!eglChooseConfig(display, attribs, &config, 1, &numConfigs)) {
        LOG_ERROR("eglChooseConfig() returned error %d", eglGetError());
        destroy();
        return false;
    }

    if (!eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format)) {
        LOG_ERROR("eglGetConfigAttrib() returned error %d", eglGetError());
        destroy();
        return false;
    }

    ANativeWindow_setBuffersGeometry(_window, 0, 0, format);

    if (!(surface = eglCreateWindowSurface(display, config, _window, 0))) {
        LOG_ERROR("eglCreateWindowSurface() returned error %d", eglGetError());
        destroy();
        return false;
    }
    
    if (!(context = eglCreateContext(display, config, 0, 0))) {
        LOG_ERROR("eglCreateContext() returned error %d", eglGetError());
        destroy();
        return false;
    }
    
    if (!eglMakeCurrent(display, surface, surface, context)) {
        LOG_ERROR("eglMakeCurrent() returned error %d", eglGetError());
        destroy();
        return false;
    }

    if (!eglQuerySurface(display, surface, EGL_WIDTH, &width) ||
        !eglQuerySurface(display, surface, EGL_HEIGHT, &height)) {
        LOG_ERROR("eglQuerySurface() returned error %d", eglGetError());
        destroy();
        return false;
    }

    _display = display;
    _surface = surface;
    _context = context;

    glDisable(GL_DITHER);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    glClearColor(0, 0, 0, 0);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    
    glViewport(0, 0, width, height);

    ratio = (GLfloat) width / height;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustumf(-ratio, ratio, -1, 1, 1, 10000);

    return true;
}

void Renderer::destroy() {
    LOG_INFO("Destroying context");

    eglMakeCurrent(_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(_display, _context);
    eglDestroySurface(_display, _surface);
    eglTerminate(_display);
    
    _display = EGL_NO_DISPLAY;
    _surface = EGL_NO_SURFACE;
    _context = EGL_NO_CONTEXT;

    return;
}

void Renderer::drawFrame()
{
	//~ float scale = 10.0;
	//~ float scale = 7.0;

	float scale = 1.0;

	//~ glEnable (GL_BLEND);
	//~ glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//~ glShadeModel (GL_FLAT);
	//~ glClearColor (0.0, 0.0, 0.0, 0.0);

// Accept fragment if it closer to the camera than the former one
   //~ glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Set background color to black and opaque
   //~ //glClearDepth(1.0f);                   // Set background depth to farthest
   //~ glEnable(GL_DEPTH_TEST);   // Enable depth testing for z-culling
   //~ //glDepthMask(GL_TRUE);
   //~ glDepthFunc(GL_LESS);    // Set the type of depth-test
   //~ glShadeModel(GL_SMOOTH);   // Enable smooth shading
   //~ //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);  // Nice perspective corrections
    //~ glEnable(GL_CULL_FACE);
    //~ glFrontFace(GL_CW);
    //~ glCullFace(GL_FRONT);

glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glColor4f(0.0f, 0.0f, 0.0f, 0.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -5.0f);
    glRotatef(_angle, 0, 1, 0);
    glRotatef(_angle*0.25f, 1, 0, 0);
    glScalef(scale,scale,scale);
	

	glBindTexture( GL_TEXTURE_2D, 0 );

	glEnable(GL_BLEND);

	glEnableClientState (GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, rossiniTexels);

    //~ glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_FLOAT, 0, COLORS);    
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, rossiniVerts);
	//glNormalPointer(GL_FLOAT, 0, rossiniNormals);
	//glTexCoordPointer(2, GL_FLOAT, 0, rossiniTexCoords);
    
    glDrawArrays(GL_TRIANGLES, 0, rossiniNumVerts);
    //~ glDrawArrays(GL_LINES, 0, rossiniNumVerts);

    _angle += 0.2f;

//	// set input data to arrays
//	glVertexPointer(3, GL_FLOAT, 0, rossiniVerts);
//	glNormalPointer(GL_FLOAT, 0, rossiniNormals);
//	glTexCoordPointer(2, GL_FLOAT, 0, rossiniTexCoords);
//
//	glEnableClientState(GL_VERTEX_ARRAY);
//	glEnableClientState(GL_COLOR_ARRAY);
//	glEnableClientState (GL_TEXTURE_COORD_ARRAY);
//
//
//	// draw data
//	glDrawArrays(GL_TRIANGLES, 0, rossiniNumVerts);
//
//	// deactivate vertex arrays after drawing
//
//
//	glDisableClientState (GL_TEXTURE_COORD_ARRAY);
//	glDisableClientState(GL_COLOR_ARRAY);
//	glDisableClientState(GL_VERTEX_ARRAY);

}

void* Renderer::threadStartCallback(void *myself)
{
    Renderer *renderer = (Renderer*)myself;

    renderer->renderLoop();
    pthread_exit(0);
    
    return 0;
}

