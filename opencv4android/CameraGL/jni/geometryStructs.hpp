#ifndef geometryStructs_hpp
#define geometryStructs_hpp

#define EPSILON 0.000001
#define maxObjSize 1596

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

void perspectiveGL( float fovY, float aspect, float zNear, float zFar );
void drawFurnish(unsigned int texName, int screenWidth, int screenHeight);
void drawBackground(unsigned int texName, int w, int h);
#endif
