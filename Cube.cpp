#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <iostream>
using namespace std;

//point on a 3D plane 
struct vec3D {
	float x, y, z;
};

//triangle defined by 3 points
struct triangle {
	vec3D p[3];
};

//structure defined by a vector, or a group of triangles. vector allows changing of size during runtime
struct mesh { 
	vector<triangle>tris;
};

//Matrix used for projection calculations
struct mat4x4 {
	float m[4][4] = { 0 };//Very important to initialize
};

class olcEngine3D : public olc::PixelGameEngine {
private:

	mesh meshCube;//instance of mesh
	mat4x4 matProj;//matrix used for projection calculations
    float fTheta;//angle value that determines rotation of cube

	//Function to multiply a vector by a matrix to give a vector
	void MultiplyMatrixVector(vec3D& i, vec3D& o, mat4x4& m) {
		o.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + m.m[3][0];
		o.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + m.m[3][1];
		o.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + m.m[3][2];
		float w;//fourth vector element that is set to 1 at the end. Used to handle multiplication by a 4x4 matrix
		w=i.x*m.m[0][3]+i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];
		//sets w to 1 by dividing vector by w. This gives us a 3D vector again
		if (w != 0) {
			o.x /= w;o.y /= w;o.z /= w;
		}
	}
public:
	olcEngine3D() {
		sAppName = "Cube";
	}
	bool OnUserCreate() override
	{
		
		//Initializer list to create a cube of dimensions 1x1x1 with a vertex at 0,0,0
		meshCube.tris = {
			// Each face of a cube is consisted of 2 traingles. Each of those triangles are
			// defined by 3 vertices. Thus we can define our cube as follow:
			
			//South face
			{0.0f,0.0f,0.0f,	0.0f,1.0f,0.0f,		1.0f,1.0f,0.0f},
			{0.0f,0.0f,0.0f,	1.0f,1.0f,0.0f,		1.0f,0.0f,0.0f},
			//East face
			{1.0f,0.0f,0.0f,	1.0f,1.0f,0.0f,		1.0f,1.0f,1.0f},
			{1.0f,0.0f,0.0f,	1.0f,1.0f,1.0f,		1.0f,0.0f,1.0f},
			//North face
			{1.0f,0.0f,1.0f,	1.0f,1.0f,1.0f,		0.0f,1.0f,1.0f},
			{1.0f,0.0f,1.0f,	0.0f,1.0f,1.0f,		0.0f,0.0f,1.0f},
			//West face
			{0.0f,0.0f,1.0f,	0.0f,1.0f,1.0f,		0.0f,1.0f,0.0f},
			{0.0f,0.0f,1.0f,	0.0f,1.0f,0.0f,		0.0f,0.0f,0.0f},
			//Top face
			{0.0f,1.0f,0.0f,	0.0f,1.0f,1.0f,		1.0f,1.0f,1.0f},
			{0.0f,1.0f,0.0f,	1.0f,1.0f,1.0f,		1.0f,1.0f,0.0f},
			//Bottom face
			{1.0f,0.0f,1.0f,	0.0f,0.0f,1.0f,		0.0f,0.0f,0.0f},
			{1.0f,0.0f,1.0f,	0.0f,0.0f,0.0f,		1.0f,0.0f,0.0f},
		};

		//Applying Projection 

		float fNear = 0.1f;//Screen from user
		float fFar = 1000.0f;//Object from screen
		float fFov = 90.0f;//Field of view
		float fAspectRatio = (float)ScreenHeight() / (float)ScreenWidth();//Aspect ratio
		float fFovRad = 1.0f / tanf(fFov * 0.5f / 180.0f * 3.14159f);//For tangent calculations in radians

		//Setting these values in our projection matrix
		matProj.m[0][0] = fAspectRatio * fFovRad;
		matProj.m[1][1] = fFovRad;
		matProj.m[2][2] = fFar / (fFar - fNear);
		matProj.m[3][2] = (-fFar * fNear) / (fFar - fNear);
		matProj.m[2][3] = 1.0f;
		matProj.m[3][3] = 0.0f;


		return true;
	}
	bool OnUserUpdate(float fElapsedTime)override {

		FillRect(0, 0, ScreenWidth(), ScreenHeight(), olc::BLACK);

		mat4x4 matRotZ, matRotX;// Matricess used to rotate cube, so that we can fully realize it
		fTheta += 1.0f * fElapsedTime;//fTheta is constantly changing as a result of using Elapsed Time

		
		//Rotation Z
		matRotZ.m[0][0] = cosf(fTheta);
		matRotZ.m[0][1] = sinf(fTheta);
		matRotZ.m[1][0] = -sinf(fTheta);
		matRotZ.m[1][1] = cosf(fTheta);
		matRotZ.m[2][2] = 1;
		matRotZ.m[3][3] = 1;

		//Rotation X
		matRotX.m[0][0] =1;
		matRotX.m[1][1] = cosf(fTheta*0.5f);
		matRotX.m[1][2] = sinf(fTheta*0.5f);
		matRotX.m[2][1] = -sinf(fTheta*0.5f);
		matRotX.m[2][2] = cosf(fTheta*0.5f);
		matRotX.m[3][3] = 1;
		

		//Drawing the triangles:

		for (auto tri : meshCube.tris) {//Goes through each triangle, tri, in meshCube

			triangle triProjected;//holds result of matrix multiplication
			triangle triTranslated;//hold translated matrix
			triangle triRotatedZ;//hold Z rotated triangle
			triangle triRotatedZX;//holds X rotated Z rotated triangle
			
			//Rotate triangle along Z axis
			MultiplyMatrixVector(tri.p[0], triRotatedZ.p[0], matRotZ);
			MultiplyMatrixVector(tri.p[1], triRotatedZ.p[1], matRotZ);
			MultiplyMatrixVector(tri.p[2], triRotatedZ.p[2], matRotZ);

			//Rotated triangle along X axis
			MultiplyMatrixVector(triRotatedZ.p[0], triRotatedZX.p[0], matRotX);
			MultiplyMatrixVector(triRotatedZ.p[1], triRotatedZX.p[1], matRotX);
			MultiplyMatrixVector(triRotatedZ.p[2], triRotatedZX.p[2], matRotX);

			//We need to Translate the cube before projecting it, so that the viewing origin may be different
			//to the cubes origin, thus we can add perspective to the cube

			triTranslated = triRotatedZX;
			triTranslated.p[0].z = triRotatedZX.p[0].z + 3.0f;
			triTranslated.p[1].z = triRotatedZX.p[1].z + 3.0f;
			triTranslated.p[2].z = triRotatedZX.p[2].z + 3.0f;



			//Projecting all vertices of current triangle, tri, and storng it in triProjected
			MultiplyMatrixVector(triTranslated.p[0], triProjected.p[0], matProj);
			MultiplyMatrixVector(triTranslated.p[1], triProjected.p[1], matProj);
			MultiplyMatrixVector(triTranslated.p[2], triProjected.p[2], matProj);

			//Right now, the matrix has been projected, but not Scaled into view. For that, We do the following:

			triProjected.p[0].x += 1.0f; triProjected.p[0].y += 1.0f;
			triProjected.p[1].x += 1.0f; triProjected.p[1].y += 1.0f;
			triProjected.p[2].x += 1.0f; triProjected.p[2].y += 1.0f;
			// now everything is between 0 and +2, so we need to half it to bring it between 0 and +1,
			//and bring it into screen width

			triProjected.p[0].x *= 0.5f * (float)ScreenWidth();
			triProjected.p[0].y *= 0.5f * (float)ScreenWidth();
			triProjected.p[1].x *= 0.5f * (float)ScreenWidth();
			triProjected.p[1].y *= 0.5f * (float)ScreenWidth();
			triProjected.p[2].x *= 0.5f * (float)ScreenWidth();
			triProjected.p[2].y *= 0.5f * (float)ScreenWidth();
			

			

			DrawTriangle(triProjected.p[0].x, triProjected.p[0].y,
				triProjected.p[1].x, triProjected.p[1].y,
				triProjected.p[2].x, triProjected.p[2].y,
				olc::Pixel(rand()% 255, rand() % 255, rand() % 255)
			);

			
		}
		return true;
	}

};
int main()
{
	olcEngine3D demo;
	if (demo.Construct(1024, 720, 2, 2))
		demo.Start();

	return 0;

}