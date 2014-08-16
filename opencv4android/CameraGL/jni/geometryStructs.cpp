

#include <stdint.h>
#include <unistd.h>
#include <iostream>
#include <EGL/egl.h> // requires ndk r5 or newer
#include <GLES/gl.h>
#include <cstdlib>
#include <cmath>
#include <algorithm>    // std::sort
#include <vector>
#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <fstream>
#include <sstream>
#include "logger.h"
#include "geometryStructs.hpp"
#include "couch.h"
#include "rossiniColors.h"

// Utility for logging:
#define LOG_TAG    "CAMERA_RENDERER"
#define LOG(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

float outNormals[maxObjSize*3];
float outTexCoords[maxObjSize*2];
float outVertexes[maxObjSize*3];
float outColors[maxObjSize][4];
float _angle = 0.0;
point3 cameraOrigin;
unsigned finalVertexSize;

int gWidth = 1024;
int gHeight = 768;
cv::Mat rgbFrame;
float deltaZcamera = 0.5;

//~ bool isFaceDrawn[87360];
cv::Mat m_furnishImage;
bool m_isTextureInitialized = false;
bool m_isFurnishTextureInitialized = false;
static GLuint texName;

//~ 
//~ void applyOrtho(float left, float right,float bottom, float top,float near, float far) const{
	//~ float a = 2.0f / (right - left);
	//~ float b = 2.0f / (top - bottom);
	//~ float c = -2.0f / (far - near);
//~ 
	//~ float tx = - (right + left)/(right - left);
	//~ float ty = - (top + bottom)/(top - bottom);
	//~ float tz = - (far + near)/(far - near);
//~ 
	//~ float ortho[16] = {
		//~ a, 0, 0, 0,
		//~ 0, b, 0, 0,
		//~ 0, 0, c, 0,
		//~ tx, ty, tz, 1
	//~ };
//~ 
//~ 
	//~ GLint projectionUniform = glGetUniformLocation(_shaderProgram, "Projection");
	//~ glUniformMatrix4fv(projectionUniform, 1, 0, &ortho[0]);
//~ }

void orthogonalStart()
{
	int w1 = gWidth;
	int h1 = gHeight;
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    //~ gluOrtho2D(-w1/2, w1/2, -h1/2, h1/2);
    glOrthof(-w1/2, w1/2, -h1/2, h1/2,-1,1);
    glMatrixMode(GL_MODELVIEW);
}

void orthogonalEnd()
{
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void perspectiveGL( float fovY, float aspect, float zNear, float zFar )
{
	//~ float aspect = screenWidth / screenHeight;
	float bt = (float) tan(45 / 2);
	float lr = bt * aspect;
	glFrustumf(-lr * 0.1f, lr * 0.1f, -bt * 0.1f, bt * 0.1f, 0.1f, 100.0f);
	
    //~ const float pi = 3.1415926535897932384626433832795;
    //~ float fW, fH;

    //fH = tan( (fovY / 2) / 180 * pi ) * zNear;
    //~ fH = (float) tan( fovY / 360 * pi ) * zNear;
    //~ fH = (float) tan( fovY / 2 ) * zNear;
    //~ fW = fH * aspect *zNear;
//~ 
    //~ glFrustumf( -fW, fW, -fH, fH, zNear, zFar );
}

void startTexture(GLuint texName){
	//~ glClear(GL_DEPTH_TEST);
	// Enable texture mapping stuff
	//~ glPushAttrib(GL_TEXTURE_BIT);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texName);
}

void endTexture(){
	glDisable(GL_TEXTURE_2D);
	//~ glPopAttrib(); // texture bit
	glBindTexture(GL_TEXTURE_2D, 0);
}

void startArrays(){
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

void endArrays(){
	glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
	glDisableClientState (GL_TEXTURE_COORD_ARRAY);
}

// for ordering from the farthest to nearest
bool nearCamera(face a, face b){
	return a.d > b.d;
};

float getDistance(point3 a, point3 b){
	return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y) + (a.z - b.z) * (a.z - b.z));
}


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

void getTextureBinders(int textureSize, unsigned textureNames[],int textureFirsts[],int textureCounts[],unsigned outTexBinders[]){
	unsigned faceSize = 0;
	// be sure that materials size and texture size are equal in original obj files
	for(unsigned i = 0; i < textureSize; ++i)
		for(unsigned j = textureFirsts[i]/3; j < (textureFirsts[i] + textureCounts[i])/3; ++j)
			outTexBinders[j] = textureNames[i+1];
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
	
	std::sort(faces.begin(),faces.end(),nearCamera);
	
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

void getAllSortedFaces(unsigned vertexesSize, point3 cameraOrigin,float inTexcoords[], float inVertexes[],
								float outTexCoords[], float outVertexes[], unsigned *finalVertexes){
	std::vector<face> faces;
	
	unsigned saved = 0;

	// we divide vertexes size by 3 because each face has 3 vertexes
	for(unsigned i  = 0; i < vertexesSize/3; ++i){
		
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
		
		point3 faceCenter;
		faceCenter.x = (vertexCoord1.x + vertexCoord2.x + vertexCoord3.x)/3.0;
		faceCenter.y = (vertexCoord1.y + vertexCoord2.y + vertexCoord3.y)/3.0;
		faceCenter.z = (vertexCoord1.z + vertexCoord2.z + vertexCoord3.z)/3.0;		

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
	
	std::sort(faces.begin(),faces.end(),nearCamera);
	
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

void getAllSortedFacesMT(unsigned vertexesSize, point3 cameraOrigin,float inTexcoords[], float inVertexes[],
						float inNormals[], float outTexCoords[], float outVertexes[], float outNormals[], unsigned *finalVertexes,
						unsigned inTexBinder[], unsigned outTexBinder[]){
	std::vector<face> faces;
	
	unsigned saved = 0;

	// we divide vertex size over 3 because each face has 3 vertexes
	for(unsigned i  = 0; i < vertexesSize/3; ++i){
		
		unsigned nineI = 9*i;
		point3 vertexCoord1;
		vertexCoord1.x = inVertexes[nineI];
		vertexCoord1.y = inVertexes[nineI + 1];
		vertexCoord1.z = inVertexes[nineI + 2];
		
		point3 vertexCoord2;
		vertexCoord2.x = inVertexes[nineI + 3];
		vertexCoord2.y = inVertexes[nineI + 4];
		vertexCoord2.z = inVertexes[nineI + 5];
		
		point3 vertexCoord3;
		vertexCoord3.x = inVertexes[nineI + 6];
		vertexCoord3.y = inVertexes[nineI + 7];
		vertexCoord3.z = inVertexes[nineI + 8];
		
		point3 faceCenter;
		faceCenter.x = (vertexCoord1.x + vertexCoord2.x + vertexCoord3.x)/3.0;
		faceCenter.y = (vertexCoord1.y + vertexCoord2.y + vertexCoord3.y)/3.0;
		faceCenter.z = (vertexCoord1.z + vertexCoord2.z + vertexCoord3.z)/3.0;		

		face currFace;
		currFace.d = getDistance(faceCenter,cameraOrigin);
		
		currFace.f.p1.x = inVertexes[nineI];
		currFace.f.p1.y = inVertexes[nineI + 1];
		currFace.f.p1.z = inVertexes[nineI + 2];
		currFace.f.p2.x = inVertexes[nineI + 3];
		currFace.f.p2.y = inVertexes[nineI + 4];
		currFace.f.p2.z = inVertexes[nineI + 5];
		currFace.f.p3.x = inVertexes[nineI + 6];
		currFace.f.p3.y = inVertexes[nineI + 7];
		currFace.f.p3.z = inVertexes[nineI + 8];
		
		currFace.n.p1.x = inNormals[nineI];
		currFace.n.p1.y = inNormals[nineI + 1];
		currFace.n.p1.z = inNormals[nineI + 2];
		currFace.n.p2.x = inNormals[nineI + 3];
		currFace.n.p2.y = inNormals[nineI + 4];
		currFace.n.p2.z = inNormals[nineI + 5];
		currFace.n.p3.x = inNormals[nineI + 6];
		currFace.n.p3.y = inNormals[nineI + 7];
		currFace.n.p3.z = inNormals[nineI + 8];
		
		unsigned sixI = 6*i;
		currFace.t.p1.x = inTexcoords[sixI];
		currFace.t.p1.y  = inTexcoords[sixI + 1];
		currFace.t.p1.z  = inTexcoords[sixI + 2];
		currFace.t.p2.x  = inTexcoords[sixI + 3];
		currFace.t.p2.y  = inTexcoords[sixI + 4];
		currFace.t.p2.z  = inTexcoords[sixI + 5];
		
		currFace.textureName = inTexBinder[i]; // face size

		faces.push_back(currFace);
		++saved;
	}
	
	std::sort(faces.begin(),faces.end(),nearCamera);
	
	std::vector<face>::iterator itFaces;
	int k = 0;
	for(itFaces = faces.begin(); itFaces < faces.end(); ++itFaces){
		unsigned nineK = 9*k;
		outVertexes[nineK]=(*itFaces).f.p1.x;
		outVertexes[nineK + 1]=(*itFaces).f.p1.y;
		outVertexes[nineK + 2]=(*itFaces).f.p1.z;
		outVertexes[nineK + 3]=(*itFaces).f.p2.x;
		outVertexes[nineK + 4]=(*itFaces).f.p2.y;
		outVertexes[nineK + 5]=(*itFaces).f.p2.z;
		outVertexes[nineK + 6]=(*itFaces).f.p3.x;
		outVertexes[nineK + 7]=(*itFaces).f.p3.y;
		outVertexes[nineK + 8]=(*itFaces).f.p3.z;
		
		outNormals[nineK]=(*itFaces).n.p1.x;
		outNormals[nineK + 1]=(*itFaces).n.p1.y;
		outNormals[nineK + 2]=(*itFaces).n.p1.z;
		outNormals[nineK + 3]=(*itFaces).n.p2.x;
		outNormals[nineK + 4]=(*itFaces).n.p2.y;
		outNormals[nineK + 5]=(*itFaces).n.p2.z;
		outNormals[nineK + 6]=(*itFaces).n.p3.x;
		outNormals[nineK + 7]=(*itFaces).n.p3.y;
		outNormals[nineK + 8]=(*itFaces).n.p3.z;
		
		unsigned sixK=6*k;
		outTexCoords[sixK]=(*itFaces).t.p1.x;
		outTexCoords[sixK + 1]=(*itFaces).t.p1.y;
		outTexCoords[sixK + 2]=(*itFaces).t.p1.z;
		outTexCoords[sixK + 3]=(*itFaces).t.p2.x;
		outTexCoords[sixK + 4]=(*itFaces).t.p2.y;
		outTexCoords[sixK + 5]=(*itFaces).t.p2.z;
		
		outTexBinder[k] = (*itFaces).textureName;
		
		++k;
	}

	*finalVertexes = 3*saved;
}



void getShadeFaces(unsigned vertexesSize, point3 lightOrigin, float inVertexes[], float outVertexes[], unsigned *finalVertexes){
	std::vector<face> faces;
	
	unsigned saved = 0;

	// we divide vertexes size by 3 because each face has 3 vertexes
	for(unsigned i  = 0; i < vertexesSize/3; ++i){
		
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

		face currFace;
		
		currFace.f.p1.x = inVertexes[9*i];
		currFace.f.p1.y = inVertexes[9*i + 1];
		currFace.f.p1.z = inVertexes[9*i + 2];
		currFace.f.p2.x = inVertexes[9*i + 3];
		currFace.f.p2.y = inVertexes[9*i + 4];
		currFace.f.p2.z = inVertexes[9*i + 5];
		currFace.f.p3.x = inVertexes[9*i + 6];
		currFace.f.p3.y = inVertexes[9*i + 7];
		currFace.f.p3.z = inVertexes[9*i + 8];

		faces.push_back(currFace);
		++saved;
	}
	
	std::sort(faces.begin(),faces.end(),nearCamera);
	
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
		
		++k;
	}

	*finalVertexes = 3*saved;
}

void scaling(float scale, float inVertexes[], float outVertexes[], unsigned vertexesSize){
	for(unsigned i  = 0; i < vertexesSize*3; ++i){
		outVertexes[i] = inVertexes[i]*scale;
	}
}

void getBackgroundTextures(int texName, int bufferIndex, cv::Mat captureBuffer[],unsigned capFrameWidth,unsigned capFrameHeight) {
	int w = capFrameWidth;
	int h = capFrameHeight;

	if(bufferIndex > 0){
		//~ pthread_mutex_lock(&FGmutex);
		cvtColor(captureBuffer[(bufferIndex - 1) % 30], rgbFrame, CV_BGR2RGB);
		//~ pthread_mutex_unlock(&FGmutex);
		w=rgbFrame.cols;
		h=rgbFrame.rows;
		//LOG_INFO("w,h, channels= %d,%d, %d, %d ",w,h,rgbFrame.channels(), textureId[0]);
		
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glBindTexture(GL_TEXTURE_2D, texName);

		if (texName != 0){
			// Upload new texture data:
		if (rgbFrame.channels() == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, rgbFrame.data);
		else if(rgbFrame.channels() == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbFrame.data);
		else if (rgbFrame.channels()==1)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, rgbFrame.data);
		}
		
		if (bufferIndex==30)
			bufferIndex = 0;
	}
}

void getFurnishTexture(unsigned int texName){
	if (!m_isFurnishTextureInitialized)
	{
		//texture image
		cv::Mat m_furnishImage = cv::imread("sdcard/Models/couch.jpg");
		cvtColor(m_furnishImage,m_furnishImage,CV_BGR2RGB);
		
		//~ //glGenTextures(1, &texName);
		glBindTexture(GL_TEXTURE_2D, texName);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);// GL_LINEAR
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

		m_isFurnishTextureInitialized = true;
	}	
}

void getObjectTexture(unsigned int texName, const cv::Mat& image){
	glBindTexture(GL_TEXTURE_2D, texName);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // GL_LINEAR
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);// GL_LINEAR
	int w = image.cols;
	int h = image.rows;

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glBindTexture(GL_TEXTURE_2D, texName);

	// Upload new texture data:
	if (image.channels() == 3)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image.data);
	else if(m_furnishImage.channels() == 4)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data);
	else if (image.channels()==1)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, image.data);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void drawFrame()
{
	//~ float scale = 10.0;
	float scale = 10.0;

 //~ // Enable texture mapping stuff
glEnable(GL_TEXTURE_2D);
glEnableClientState(GL_TEXTURE_COORD_ARRAY);
glDisableClientState(GL_COLOR_ARRAY);


    glEnableClientState(GL_VERTEX_ARRAY);
	//~ glColor4f(0.0f, 0.0f, 0.0f, 0.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //~ glPushMatrix();
    glTranslatef(0, 0, -5.0f);
    
    glRotatef(_angle, 0, 1, 0);

    glScalef(scale,scale,scale);
    
    GLfloat mdl[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, mdl);
	getCameraOrigin(mdl,&cameraOrigin);
	//LOG_INFO("x=%f y=%f z=%f",cameraOrigin.x,cameraOrigin.y,cameraOrigin.z);
	//~ getFacesToCamera(couchNumVerts,cameraOrigin,couchTexCoords,COLORS,couchVerts,
	getFacesNearToCamera(couchNumVerts,cameraOrigin,couchTexCoords,COLORS,couchVerts,
	outTexCoords,outColors,outVertexes,&finalVertexSize);

    glVertexPointer(3, GL_FLOAT, 0, outVertexes);
	glTexCoordPointer(2, GL_FLOAT, 0, outTexCoords);

	glDrawArrays(GL_TRIANGLES, 0, finalVertexSize);


    _angle += 0.2f;

	glDisableClientState (GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
    //~ glDisableClientState(GL_COLOR_ARRAY);
    //~ glDisableClientState(GL_NORMAL_ARRAY);
    glEnable(GL_TEXTURE_2D);
	glFlush();
}

void drawBackground(unsigned int texName, int w, int h){
	//~ int w = captureBuffer[0].cols;
	//~ int h = captureBuffer[0].rows;
	orthogonalStart();
  const GLfloat bgTextureVertices[] = { 0, 0, w, 0, 0, h, w, h };
  const GLfloat bgTextureCoords[]   = { 1, 0, 1, 1, 0, 0, 0, 1 };
  const GLfloat proj[]              = { 0, -2.0f/w, 0, 0, -2.0f/h, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1 };

	LOG_INFO("w,h, channels= %d,%d ",w,h);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(proj);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	startTexture(texName);
	startArrays();

	glVertexPointer(2, GL_FLOAT, 0, bgTextureVertices);
	glTexCoordPointer(2, GL_FLOAT, 0, bgTextureCoords);

	glColor4f(1,1,1,1);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	endArrays();
	endTexture();
	//~ glBindTexture(GL_TEXTURE_2D, 0);
	orthogonalEnd();
}

void drawFurnishTexture(unsigned int texName, int w, int h){
	//~ int w = captureBuffer[0].cols;
	//~ int h = captureBuffer[0].rows;
  const GLfloat bgTextureVertices[] = { 0, 0, w, 0, 0, h, w, h };
  const GLfloat bgTextureCoords[]   = { 1, 0, 1, 1, 0, 0, 0, 1 };
  const GLfloat proj[]              = { 0, -2.0f/w, 0, 0, -2.0f/h, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1 };

	LOG_INFO("w,h, channels= %d,%d ",w,h);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(proj);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texName);

	// Update attribute values.
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	glVertexPointer(2, GL_FLOAT, 0, bgTextureVertices);
	glTexCoordPointer(2, GL_FLOAT, 0, bgTextureCoords);

	glColor4f(1,1,1,1);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_TEXTURE_2D);
	//~ glBindTexture(GL_TEXTURE_2D, 0);
}

void drawFurnish(unsigned int texName, int screenWidth, int screenHeight)
{
	//~ orthogonalStart();
	//~ drawBackground(texName, screenWidth, screenHeight);
	//~ orthogonalEnd();
	
	float scale = 2.0;

	//~ glColor4f(1.0f,1.0f,1.0f,1.0f);
	// Enable texture mapping stuff
	startTexture(texName);
	startArrays();
    
    float aspect = screenWidth / screenHeight;
    float fovY =628.7519411113429;
	//~ float bt = (float) tan(fy / 2);
	//~ float lr = bt * aspect;
	//~ glFrustumf(-lr * 0.1f, lr * 0.1f, -bt * 0.1f, bt * 0.1f, 0.1f, 100.0f);
    const float pi = 3.1415926535897932384626433832795;
    float fW, fH, zNear = 0.01, zFar = 100;

    //~ //fH = tan( (fovY / 2) / 180 * pi ) * zNear;
    fH = (float) (tan( (fovY / 360) * pi ) * zNear);
    //~ fH = (float) tan( fovY / 2 ) * zNear;
    fW = fH * aspect *zNear;

    glFrustumf( -fW, fW, -fH, fH, zNear, zFar );
    //~ perspectiveGL(45, (float)screenWidth/ screenHeight, 0.1, 100);
    //~ glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //~ glTranslatef(0.0, 0.0, -0.803407f);
    glTranslatef(0.0, 0.0, -0.8);
    glRotatef(_angle, 0, 0, 1);
    
    //~ glScalef(scale,scale,scale);
    
    GLfloat mdl[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, mdl);
	getCameraOrigin(mdl,&cameraOrigin);
	
	LOG_INFO("x=%f y=%f z=%f, angle=%f",cameraOrigin.x,cameraOrigin.y,cameraOrigin.z,_angle);
	cameraOrigin.x *= scale;
	cameraOrigin.y *= scale;
	cameraOrigin.z *= scale;
	getFacesNearToCamera(couchNumVerts,cameraOrigin,couchTexCoords,COLORS,couchVerts,
	outTexCoords,outColors,outVertexes,&finalVertexSize);
	//~ finalVertexSize = couchNumVerts;
	//~ for(unsigned int i = 0; i < maxObjSize*3; ++i)
		//~ outVertexes[i] = couchVerts[i];
	//~ for(unsigned int i = 0; i < maxObjSize*2; ++i)
		//~ outTexCoords[i] = couchTexCoords[i];
	
	// scaling the vertexes withon any help
	scaling(scale, outVertexes, outVertexes,finalVertexSize);
	
	glVertexPointer(3, GL_FLOAT, 0, outVertexes);
    glTexCoordPointer(2, GL_FLOAT, 0, outTexCoords);
	
	glDrawArrays(GL_TRIANGLES, 0, finalVertexSize);
	//~ glTranslatef(0.0, 0.0, 0.803407f);
	//~ glRotatef(-_angle, 0, 0, 1);
	LOG_INFO("Furnish completed");
	//~ glTranslatef(0.0, 0.0, 3.0);
	endArrays();
    endTexture();
    //~ glPopMatrix();
     _angle += 0.2;
}

void drawCameraFrame()
{
	orthogonalStart();
	glColor4f(1.0f,1.0f,1.0f,1.0f);

	//~ int w = m_backgroundImage.cols;
	int w = m_furnishImage.cols;
	//~ int h = m_backgroundImage.rows;
	int h = m_furnishImage.rows;

	const GLfloat bgTextureVertices[] = { 0, 0, w, 0, 0, h, w, h };
	const GLfloat bgTextureCoords[]   = { 1, 0, 1, 1, 0, 0, 0, 1 };
	const GLfloat proj[]              = { 0, -2.f/w, 0, 0, -2.f/h, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1 };

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(proj);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texName);

	// Update attribute values.
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	
	GLfloat mdl[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, mdl);
	getCameraOrigin(mdl,&cameraOrigin);

	glVertexPointer(2, GL_FLOAT, 0, bgTextureVertices);
	glTexCoordPointer(2, GL_FLOAT, 0, bgTextureCoords);

	glColor4f(1,1,1,1);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_TEXTURE_2D);
	orthogonalEnd();
};
