#include <jni.h>
#include <cv.h>
#include <EGL/egl.h> // requires ndk r5 or newer
#include <GLES/gl.h>
#include <stdlib.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "NativeLibrary.h"
#include "couch.h"
#include "rossiniColors.h"
#include "rossini3.h"
#include "logger.h"
#define LOG_TAG "EglSample"

bool mIsTextureInitialized = false;
unsigned finalVertexSize;
static GLuint texName;
//~ #define maxObjSize 1596
#define maxObjSize 87360
float outNormals[maxObjSize*3];
float outTexCoords[maxObjSize*2];
float outVertexes[maxObjSize*3];
float outColors[maxObjSize][4];

//~ /*  Create checkerboard texture  */
//~ #define checkImageWidth 64
//~ #define checkImageHeight 64
//~ static GLubyte checkImage[checkImageHeight][checkImageWidth][4];

struct point3{
	float x;
	float y;
	float z;
};

struct triangle{
	point3 p1;
	point3 p2;
	point3 p3;
};

struct texture{
	point3 p1;
	point3 p2;
};

struct face{
	triangle f;
	texture t;
	float d; // distance
};

// for ordering from the farthest to nearest
bool farToCamera(face a, face b){
	return a.d > b.d;
};

float getDistance(point3 a, point3 b){
	return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y) + (a.z - b.z) * (a.z - b.z));
}

point3 cameraOrigin;

double internalProduct(const point3 *a, const point3 *b){
	return (*a).x * (*b).x + (*a).y * (*b).y + (*a).z * (*b).z;
}

point3 vectorProduct(const point3 *a, const point3 *b){
	point3 c;
	c.x = (*a).y * (*b).z - (*a).z * (*b).y;
	c.y = -(*a).x * (*b).z + (*a).z * (*b).x;
	c.z = (*a).x * (*b).y - (*a).y * (*b).x;
	return c;
}

// vector comes from a and goes to b
point3 directedVector(point3 *a, point3 * b){
	
	point3 a2b;
	a2b.x = (*b).x - (*a).x;
	a2b.y = (*b).y - (*a).y;
	a2b.z = (*b).z - (*a).z;
	
	return a2b;
}
#define EPSILON 1e-7

int intersectTriangle(point3 *orig, point3 *dir, point3 *v1, point3 *v2, point3 *v3,
		float *t, float *u, float *v){
	point3 e1, e2, tV, pV, qV;
	float det, invDet;
	
	// find vectors for two edges sharing v2
	e1 = directedVector(v1,v2);
	e2 = directedVector(v1,v3);
	
	// begin calculating determinant - also used to calculate U parameter
	pV = vectorProduct(dir,&e2);
	
	//if determinant is near zero, ray lies in plane of triangle
	det = internalProduct(&e1,&pV);
	
	if(det > -EPSILON && det < EPSILON)
		return 0;
		
	invDet = 1.0 / det;
	//calculate distance from v1 to ray origin
	tV = directedVector(v1, orig);
	
	// calculate U parameter and test bounds
	*u = internalProduct(&tV, &pV) * invDet;
	
	if(*u < 0.0 || *u > 1.0)
		return 0;
		
	// prepare to test V parameter
	qV = vectorProduct(&tV,&e1);
		
	// calculate V parameter and test bounds
	*v = internalProduct(dir,&qV) * invDet;
	if(*v < 0.0 || *u + *v > 1.0)
		return 0;
	
	// calculate t, ray intersects triangle 
	*t = internalProduct(&e2, &qV) * invDet;
	
	return 1;
}

int intersectFace(point3 *orig, point3 *dir, point3 *v1, point3 *v2, point3 *v3){
	//~ float t;
	float u, v;
	point3 e1, e2, tV, pV, qV;
	float det, invDet;
	
	// find vectors for two edges sharing v2
	e1 = directedVector(v1,v2);
	e2 = directedVector(v1,v3);
	
	// begin calculating determinant - also used to calculate U parameter
	pV = vectorProduct(dir,&e2);
	
	//if determinant is near zero, ray lies in plane of triangle
	det = internalProduct(&e1,&pV);
	
	if(det > -EPSILON && det < EPSILON)
		return 0;
		
	invDet = 1.0 / det;
	//calculate distance from v1 to ray origin
	tV = directedVector(v1, orig);
	
	// calculate U parameter and test bounds
	u = internalProduct(&tV, &pV) * invDet;
	
	if(u < 0.0 || u > 1.0)
		return 0;
		
	// prepare to test V parameter
	qV = vectorProduct(&tV,&e1);
		
	// calculate V parameter and test bounds
	v = internalProduct(dir,&qV) * invDet;
	if(v < 0.0 || u + v > 1.0)
		return 0;
	
	// calculate t, ray intersects triangle 
	//~ t = internalProduct(&e2, &qV) * invDet;
	
	return 1;
}
 
void getCameraOrigin(GLfloat mdl[16], point3 *camera_org){
    
    (*camera_org).x = -(mdl[0] * mdl[12] + mdl[1] * mdl[13] + mdl[2] * mdl[14]);
    (*camera_org).y = -(mdl[4] * mdl[12] + mdl[5] * mdl[13] + mdl[6] * mdl[14]);
    (*camera_org).z = -(mdl[8] * mdl[12] + mdl[9] * mdl[13] + mdl[10] * mdl[14]);

}

void getFacesNearToCamera(unsigned vertexesSize, point3 cameraOrigin,float inTexcoords[], float inColors[][4], float inVertexes[],
								float outTexCoords[], float outColors[][4], float outVertexes[], unsigned *finalVertexes){
	std::vector<face> faces;
	
	unsigned saved = 0;

	// we divide vertexes size by 3 because each face has 3 vertexes
	for(unsigned i  = 0; i < vertexesSize/3; ++i){

		// we realize the vector product according to hand right rule
		
		point3 vertexCoord1;
		vertexCoord1.x = inVertexes[9*i];
		vertexCoord1.y = inVertexes[9*i + 1];
		vertexCoord1.z = inVertexes[9*i + 2];
		
		point3 vertexCoord2;
		vertexCoord2.x = inVertexes[9*i + 3];
		vertexCoord2.y = inVertexes[9*i + 4];
		vertexCoord2.z = inVertexes[9*i + 5];
		
		point3 vertexCoord3;
		vertexCoord3.x = inVertexes[9*i + 6];
		vertexCoord3.y = inVertexes[9*i + 7];
		vertexCoord3.z = inVertexes[9*i + 8];

		// this vector comes from p2 and goes to p1
		point3 vT1 = directedVector(&vertexCoord2,&vertexCoord1);
		// this vector comes from p2 and goes to p3
		point3 vT2 = directedVector(&vertexCoord2,&vertexCoord3);
		// we realize the vector product according to hand right rule
		point3 normalOutFace = vectorProduct(&vT2,&vT1);
		
		point3 faceCenter;
		faceCenter.x = (vertexCoord1.x + vertexCoord2.x + vertexCoord3.x)/3.0;
		faceCenter.y = (vertexCoord1.y + vertexCoord2.y + vertexCoord3.y)/3.0;
		faceCenter.z = (vertexCoord1.z + vertexCoord2.z + vertexCoord3.z)/3.0;
		// vector from triangle face to camera origin;
		point3 tF2cO = directedVector(&faceCenter,&cameraOrigin);

		// calcule internal product between gC2TC and normalOutFace
		// if internal product greater or equal than zero 
		//   save the coords that are facing to camera
		//~ if (internalProduct(&tF2cO,&normalOutFace) > 0){
		
		if (internalProduct(&tF2cO,&normalOutFace) > EPSILON){

			point3 dir = directedVector(&cameraOrigin,&faceCenter);
			
			if (intersectFace(&cameraOrigin, &dir, &vertexCoord1, &vertexCoord2, &vertexCoord3) == 1) {
			//~ int k = saved;
			face currFace;
			
			currFace.d = getDistance(faceCenter,cameraOrigin);
			//~ outColors[3*k][0] = inColors[3*i][0];
			//~ outColors[3*k][1] = inColors[3*i][1];
			//~ outColors[3*k][2] = inColors[3*i][2];
			//~ outColors[3*k][3] = inColors[3*i][3];
			//~ outColors[3*k + 1][0] = inColors[3*i + 1][0];
			//~ outColors[3*k + 1][1] = inColors[3*i + 1][1];
			//~ outColors[3*k + 1][2] = inColors[3*i + 1][2];
			//~ outColors[3*k + 1][3] = inColors[3*i + 1][3];
			//~ outColors[3*k + 2][0] = inColors[3*i + 2][0];
			//~ outColors[3*k + 2][1] = inColors[3*i + 2][1];
			//~ outColors[3*k + 2][2] = inColors[3*i + 2][2];
			//~ outColors[3*k + 2][3] = inColors[3*i + 2][3];
			
			currFace.f.p1.x = inVertexes[9*i];
			currFace.f.p1.y = inVertexes[9*i + 1];
			currFace.f.p1.z = inVertexes[9*i + 2];
			currFace.f.p2.x = inVertexes[9*i + 3];
			currFace.f.p2.y = inVertexes[9*i + 4];
			currFace.f.p2.z = inVertexes[9*i + 5];
			currFace.f.p3.x = inVertexes[9*i + 6];
			currFace.f.p3.y = inVertexes[9*i + 7];
			currFace.f.p3.z = inVertexes[9*i + 8];
			
			currFace.t.p1.x = inTexcoords[6*i];
			currFace.t.p1.y  = inTexcoords[6*i + 1];
			currFace.t.p1.z  = inTexcoords[6*i + 2];
			currFace.t.p2.x  = inTexcoords[6*i + 3];
			currFace.t.p2.y  = inTexcoords[6*i + 4];
			currFace.t.p2.z  = inTexcoords[6*i + 5];

			faces.push_back(currFace);
			++saved;
			}
		}
	}
	
	std::sort(faces.begin(),faces.end(),farToCamera);
	
	std::vector<face>::iterator itFaces;
	int k = 0;
	for(itFaces = faces.begin(); itFaces < faces.end(); ++itFaces){
		outVertexes[9*k]=(*itFaces).f.p1.x;
		outVertexes[9*k + 1]=(*itFaces).f.p1.y;
		outVertexes[9*k + 2]=(*itFaces).f.p1.z;
		outVertexes[9*k + 3]=(*itFaces).f.p2.x;
		outVertexes[9*k + 4]=(*itFaces).f.p2.y;
		outVertexes[9*k + 5]=(*itFaces).f.p2.z;
		outVertexes[9*k + 6]=(*itFaces).f.p3.x;
		outVertexes[9*k + 7]=(*itFaces).f.p3.y;
		outVertexes[9*k + 8]=(*itFaces).f.p3.z;
		
		outTexCoords[6*k]=(*itFaces).t.p1.x;
		outTexCoords[6*k + 1]=(*itFaces).t.p1.y;
		outTexCoords[6*k + 2]=(*itFaces).t.p1.z;
		outTexCoords[6*k + 3]=(*itFaces).t.p2.x;
		outTexCoords[6*k + 4]=(*itFaces).t.p2.y;
		outTexCoords[6*k + 5]=(*itFaces).t.p2.z;
		
		++k;
	}

	*finalVertexes = 3*saved;
}

void scaling(float scale, float inVertexes[], float outVertexes[], unsigned vertexesSize){
	for(unsigned i  = 0; i < vertexesSize*3; ++i){
		outVertexes[i] = inVertexes[i]*scale;
	}
}

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
    //~ glEnable(GL_CULL_FACE);

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
	//~ float scale = 10.0;
	float scale = 500.0;

	//~ float scale = 1.0;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//~ glShadeModel(GL_FLAT);
	//~ glEnable(GL_DEPTH_TEST);

	if (!mIsTextureInitialized)
	{
		//texture image
		cv::Mat m_furnishImage = cv::imread("sdcard/Models/couch.jpg");

		cvtColor(m_furnishImage,m_furnishImage,CV_BGR2RGBA);
		
		glGenTextures(1, &texName);
		glBindTexture(GL_TEXTURE_2D, texName);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		int w = m_furnishImage.cols;
		int h = m_furnishImage.rows;

		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glBindTexture(GL_TEXTURE_2D, texName);

		// Upload new texture data:
		if (m_furnishImage.channels() == 3)
		// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, m_furnishImage.data);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, m_furnishImage.data);
		else if(m_furnishImage.channels() == 4)
		// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, m_furnishImage.data);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_furnishImage.data);
		else if (m_furnishImage.channels()==1)
		// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_furnishImage.data);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_furnishImage.data);		
			
		mIsTextureInitialized = true;
	}	

	// Enable texture mapping stuff
	glEnable(GL_TEXTURE_2D);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

    glEnableClientState(GL_VERTEX_ARRAY);
	//~ glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(projection);
        
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(pose);
    //~ glMatrixMode(GL_MODELVIEW);
    //~ glLoadIdentity();

    //~ glTranslatef(0, 0, -5.0f);

    //~ glScalef(scale,scale,scale);
    
    GLfloat mdl[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, mdl);
	getCameraOrigin(mdl,&cameraOrigin);
	LOG_INFO("x=%f y=%f z=%f",cameraOrigin.x,cameraOrigin.y,cameraOrigin.z);
	getFacesNearToCamera(couchNumVerts,cameraOrigin,couchTexCoords,COLORS,couchVerts,
	outTexCoords,outColors,outVertexes,&finalVertexSize);
	
	// scaling the vertexes withon any help
	scaling(scale, outVertexes, outVertexes,finalVertexSize);

    glVertexPointer(3, GL_FLOAT, 0, outVertexes);
	glTexCoordPointer(2, GL_FLOAT, 0, outTexCoords);

	glDrawArrays(GL_TRIANGLES, 0, finalVertexSize);

	glDisableClientState (GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
    glDisable(GL_TEXTURE_2D);
	glFlush();
}
