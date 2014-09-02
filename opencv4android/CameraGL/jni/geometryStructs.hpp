#ifndef geometryStructs_hpp
#define geometryStructs_hpp

#define EPSILON 1e-12
#define maxObjSize 1596

#include <GLES/gl.h>
#include <GLES/glext.h>

struct point3{
	float x;
	float y;
	float z;
};

struct point2{
	float x;
	float y;
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
	triangle n;
	float d; // distance
	unsigned textureName;
};

struct vectorFace{
	std::vector<point3> f;
	std::vector<point2> t;
	std::vector<point3> n;
	float d; // distance
	unsigned textureName;
	unsigned vertexSize;
};

void orthogonalStart();
void orthogonalEnd();
void startArrays();
void endArrays();
void startTexture(GLuint);
void endTexture();
void perspectiveGL( float fovY, float aspect, float zNear, float zFar );
void drawFurnish(unsigned int texName, int screenWidth, int screenHeight);
void drawBackground(unsigned int texName, int w, int h);
void getFurnishTexture(unsigned int);
void getBackgroundTextures();
void getObjectTexture(unsigned int, const cv::Mat&);
void getCameraOrigin(GLfloat mdl[16], point3 *camera_org);
void getTextureBinders(int textureSize, unsigned textureNames[],int textureFirsts[],int textureCounts[],unsigned outTexBinders[]);
void getVectorTextureBinders(int bindersSize, int firstTexture, unsigned textureNames[], unsigned inTexBinders[],unsigned outTexBinders[]);
void scaling(float scale, float inVertexes[], float outVertexes[], unsigned vertexesSize);
void getFacesNearToCamera(unsigned vertexesSize, point3 cameraOrigin,float inTexcoords[], float inColors[][4], float inVertexes[],
								float outTexCoords[], float outColors[][4], float outVertexes[], unsigned *finalVertexes);
void getAllSortedFaces(unsigned vertexesSize, point3 cameraOrigin,float inTexcoords[], float inVertexes[],
								float outTexCoords[], float outVertexes[], unsigned *finalVertexes);								
void getAllSortedFacesMT(unsigned vertexesSize, point3 cameraOrigin,float inTexcoords[], float inVertexes[], float inNormals[],
								float outTexCoords[], float outVertexes[], float outNormals[], unsigned *finalVertexes,
								unsigned inTexBinder[], unsigned outTexBinder[]);
void getAllSortedVectorFacesMT(unsigned faceSize, unsigned vertexByFaceSizes[], unsigned outVertexByFaces[] ,point3 cameraOrigin,float inTexcoords[], float inVertexes[],
						float inNormals[], float outTexCoords[], float outVertexes[], float outNormals[], unsigned *finalVertexes, unsigned *finalFaces,
						unsigned inTexBinder[], unsigned outTexBinder[]);								
void getAllSortedFacesMTNoNormals(unsigned vertexesSize, point3 cameraOrigin,float inTexcoords[], float inVertexes[],
								float outTexCoords[], float outVertexes[], unsigned *finalVertexes,
								unsigned inTexBinder[], unsigned outTexBinder[]);								
float getDistance(point3 a, point3 b);
#endif
