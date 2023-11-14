/*==================================================================================
* COSC 363  Computer Graphics (2023)
* Department of Computer Science and Software Engineering, University of Canterbury.
*
* A basic ray tracer
* See Lab06.pdf   for details.
*===================================================================================
*/


#include "Plane.h"
#include "TextureBMP.h"
#include "Cylinder.h"
#include "Cone.h"
#include <random>

#include <iostream>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include "Sphere.h"
#include "SceneObject.h"
#include "Ray.h"
#include <GL/freeglut.h>
using namespace std;

const float EDIST = 40.0;
const int NUMDIV = 500;
const int MAX_STEPS = 5;
const float XMIN = -10.0;
const float XMAX = 10.0;
const float YMIN = -10.0;
const float YMAX = 10.0;
const double PI = 3.1415926535897931;



vector<SceneObject*> sceneObjects;
TextureBMP texture;
TextureBMP texture2;

//---The most important function in a ray tracer! ---------------------------------- 
//   Computes the colour value obtained by tracing a ray and finding its 
//     closest point of intersection with objects in the scene.
//----------------------------------------------------------------------------------

glm::vec3 trace(Ray ray, int step)
{
	glm::vec3 backgroundCol(0);						//Background colour = (0,0,0)

	glm::vec3 lightPos(20, 23, -40);					//Light's position
	glm::vec3 lightPos1(10, 40, -3);
	glm::vec3 lightPos2(0, 0, 20);
	glm::vec3 color(0);
	SceneObject* obj;

    ray.closestPt(sceneObjects);					//Compare the ray with all objects in the scene
    if(ray.index == -1) return backgroundCol;		//no intersection
	obj = sceneObjects[ray.index];					//object on which the closest point of intersection is found

	//---- Color for floor ------------------------------------------
	if (ray.index == 3)
	{
		//Stripe pattern
		int stripe_X = 6;
		int ix = abs(ray.hit.x) / stripe_X;

		int stripeWidth = 6;
		int iz = abs(ray.hit.z) / stripeWidth;

		int kz = iz % 2; //2 colors
		int kx = ix % 2;

		if (ray.hit.x > 0) {
			if (kx == 0) {
				kx = 1;
			}
			else {
				kx = 0;
			}
		}

		if ((kx == kz)) {
			color = glm::vec3(0.3, 0.15, 0.15);
		}
		else {
			color = glm::vec3(1, 1, 1);
		}

		obj->setColor(color);
	}


	if (ray.index == 8)
	{
		float texcoords = (ray.hit.x - (-50)) / (50 - (-50));
		float texcoordt = (ray.hit.y - (-20)) / ((40) - (-20));
		if (texcoords > 0 && texcoords < 1 &&
			texcoordt > 0 && texcoordt < 1)
		{
			color = texture2.getColorAt(texcoords, texcoordt);
			obj->setColor(color);
		}
	}

	if (ray.index == 13 || ray.index == 14 || ray.index == 15)
	{
		float stripe = 1.0f;
		int iz = ray.hit.z / stripe;
		int kz = iz % 3; //2 colors
		int kxy1 = ray.hit.x + ray.hit.y;
		int kxy2 = ray.hit.x - ray.hit.y;

		//kxy1 % 3 == kz
		if (kxy1 % 3 == kz) {
			color = glm::vec3(1, 1, 1);
		}
		else if (kxy1 % 3 == 2) {
			color = glm::vec3(1, 1, 0);
		}
		else {
			color = glm::vec3(0, 0.5, 0);
		}
		obj->setColor(color);
	}

	//---- Texture -----------------------------------------------
	if (ray.index == 0) {
		glm::vec3 center(-15.5, 0.0, -100.0);
		glm::vec3 n = glm::normalize(ray.hit - center);

		float u = 0.5 + atan2(n.z, n.x) / (2 * PI); 
		float v = 0.5 + asin(n.y) / PI; 
		
		color = texture.getColorAt(u, v);
		obj->setColor(color);
	}


	color = obj->getColor();                        //Object's colour
	if (ray.index == 4) {
		color = obj->lighting(lightPos1, -ray.dir, ray.hit);
	}
	else if (ray.index == 8) {
		color = obj->lighting(lightPos2, ray.dir, ray.hit);
	}
	else {
		color = obj->lighting(lightPos, -ray.dir, ray.hit);
	}

	//---- Shadow ------------------------------------------------
	glm::vec3 lightVec = lightPos - ray.hit;
	Ray shadowRay(ray.hit, lightVec);
	shadowRay.closestPt(sceneObjects);

	// SceneObject* Sec_obj = sceneObjects[shadowRay.index];
	float lightDist = glm::length(lightVec);


	// lighter shadows - Transparent & Refractive obj


	if (shadowRay.index > -1 && shadowRay.dist < lightDist) {
		SceneObject* Sec_obj = sceneObjects[shadowRay.index];

		if (Sec_obj->isTransparent() || Sec_obj->isRefractive()) {
			color = 0.5f * obj->getColor(); //0.8 = ambient scale factor
		}
		else {
			color = 0.2f * obj->getColor(); //0.2 = ambient scale factor
		}
	}



	//---- Reflective ----------------------------------------------
	if (obj->isReflective() && step < MAX_STEPS)
	{
		float rho = obj->getReflectionCoeff();
		glm::vec3 normalVec = obj->normal(ray.hit);
		glm::vec3 reflectedDir = glm::reflect(ray.dir, normalVec);
		Ray reflectedRay(ray.hit, reflectedDir);
		glm::vec3 reflectedColor = trace(reflectedRay, step + 1);
		color = color + (rho * reflectedColor);
	}





	//---- Transparent ------------------------------------------------
	if (obj->isTransparent())
	{
		float tho = obj->getTransparencyCoeff();

		Ray interTransparentRay(ray.hit, ray.dir);    
		interTransparentRay.closestPt(sceneObjects);					//Compare the ray with all objects in the scene

		Ray exterTransparentRay(interTransparentRay.hit, ray.dir);
		//exterTransparentRay.closestPt(sceneObjects);					//Compare the ray with all objects in the scene
		//if (exterTransparentRay.index == -1) return backgroundCol;		//no intersection
		glm::vec3 transparentColor = trace(exterTransparentRay, step + 1);
		color = color + tho * transparentColor;
	}

	
	//---- Refractive ------------------------
	if (obj->isRefractive())
	{
		glm::vec3 n = obj->normal(ray.hit);
		float eta = 1 / obj->getRefractiveIndex();
		
		glm::vec3 g = glm::refract(ray.dir, n, eta);
		Ray refrRay(ray.hit, g);
		refrRay.closestPt(sceneObjects);

		glm::vec3 m = obj->normal(refrRay.hit);
		glm::vec3 h = glm::refract(refrRay.dir, -m, 1.0f/eta);

		Ray refractedRay(refrRay.hit, h);
		glm::vec3 refractiveColor= trace(refractedRay, step + 1);
		color = color + refractiveColor;
	}



	// Fog
	float z1 = -100;
	float z2 = -500;

	if (ray.hit.z <= z1 && ray.hit.z >= z2) {
		float factor = (ray.hit.z - z1) / (z2 - z1);
		color = (1.0f - factor) * color + factor * (1.0f, 1.0f, 1.0f);
	}

	return color;
}

//---The main display module -----------------------------------------------------------
// In a ray tracing application, it just displays the ray traced image by drawing
// each cell as a quad.
//---------------------------------------------------------------------------------------
void display()
{
	float xp, yp;  //grid point
	float cellX = (XMAX - XMIN) / NUMDIV;  //cell width
	float cellY = (YMAX - YMIN) / NUMDIV;  //cell height
	glm::vec3 eye(0., 1., 0.);

	glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	glBegin(GL_QUADS);  //Each cell is a tiny quad.

	for (int i = 0; i < NUMDIV; i++)	//Scan every cell of the image plane
	{
		xp = XMIN + i * cellX;
		for (int j = 0; j < NUMDIV; j++)
		{
			yp = YMIN + j * cellY;

			//glm::vec3 dir(xp + 0.5 * cellX, yp + 0.5 * cellY, -EDIST);	//direction of the primary ray
			//Ray ray = Ray(eye, dir);
			//glm::vec3 col = trace(ray, 1);
			glm::vec3 dir1(xp + 0.25 * cellX, yp + 0.25 * cellY, -EDIST);	
			glm::vec3 dir2(xp + 0.25 * cellX, yp + 0.75 * cellY, -EDIST);
			glm::vec3 dir3(xp + 0.75 * cellX, yp + 0.25 * cellY, -EDIST);
			glm::vec3 dir4(xp + 0.75 * cellX, yp + 0.75 * cellY, -EDIST);
			Ray ray1 = Ray(eye, dir1);
			Ray ray2 = Ray(eye, dir2);
			Ray ray3 = Ray(eye, dir3);
			Ray ray4 = Ray(eye, dir4);
			glm::vec3 col1 = trace(ray1, 1); //Trace the primary ray and get the colour value
			glm::vec3 col2 = trace(ray2, 1); //Trace the primary ray and get the colour value
			glm::vec3 col3 = trace(ray3, 1); //Trace the primary ray and get the colour value
			glm::vec3 col4 = trace(ray4, 1); //Trace the primary ray and get the colour value

			 
			glm::vec3 col = (col1 + col2 + col3 + col4) / 4.0f;

			glColor3f(col.r, col.g, col.b);
			glVertex2f(xp, yp);				//Draw each cell with its color value
			glVertex2f(xp + cellX, yp);
			glVertex2f(xp + cellX, yp + cellY);
			glVertex2f(xp, yp + cellY);
		}
	}

    glEnd();
    glFlush();
}



//---This function initializes the scene ------------------------------------------- 
//   Specifically, it creates scene objects (spheres, planes, cones, cylinders etc)
//     and add them to the list of scene objects.
//   It also initializes the OpenGL 2D orthographc projection matrix for drawing the
//     the ray traced image.
//----------------------------------------------------------------------------------
void initialize()
{
	texture = TextureBMP("Christmas.bmp");
	texture2 = TextureBMP("pixel.bmp");

    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(XMIN, XMAX, YMIN, YMAX);

    glClearColor(0, 0, 0, 1);

	Sphere* sphere1 = new Sphere(glm::vec3(-15.5, 0.0, -100.0), 7);
	sphere1->setColor(glm::vec3(0, 0, 1));   //Set colour to blue
	//sphere1->setReflectivity(true, 0.8);
	sceneObjects.push_back(sphere1);		 //Add sphere to scene objects

	Sphere* sphere2 = new Sphere(glm::vec3(4, -13.0, -100.0), 6.0);
	sphere2->setColor(glm::vec3(0.0, 0.0, 0.0));   //Set colour to 
	sphere2->setTransparency(true, 0.4);
	//sphere2->setReflectivity(true, 0.1);
	//sphere2->setRefractivity(true, 1.0, 1.005);
	sceneObjects.push_back(sphere2);		 //Add sphere to scene objects

	Sphere* sphere3 = new Sphere(glm::vec3(18, -13.0, -100.0), 6.0);
	sphere3->setColor(glm::vec3(0, 0, 0));   //Set colour to 
	//sphere3->setReflectivity(true, 1);
	sphere3->setRefractivity(true, 1.0, 1.5);
	sceneObjects.push_back(sphere3);		 //Add sphere to scene objects


	
	Plane* plane = new Plane(glm::vec3(-50., -20, 10), //Point A
		glm::vec3(50., -20, 10), //Point B
		glm::vec3(50., -20, -260), //Point C
		glm::vec3(-50., -20, -260)); //Point D
	plane->setColor(glm::vec3(0.8, 0.8, 0));
	plane->setSpecularity(false);
	sceneObjects.push_back(plane);

	Plane* plane2 = new Plane(glm::vec3(-50., 40, 10), //Point A
		glm::vec3(50., 40, 10), //Point B
		glm::vec3(50., 40, -260), //Point C
		glm::vec3(-50., 40, -260)); //Point D
	plane2->setColor(glm::vec3(0, 1, 0));
	plane2->setSpecularity(false);
	sceneObjects.push_back(plane2);

	//Counterclockwise! for left side Plane, 
	Plane* plane3 = new Plane(glm::vec3(-50, -20, 10), //Point A
		glm::vec3(-50., -20, -260), //Point D
		glm::vec3(-50., 40, -260), //Point C
		glm::vec3(-50., 40, 10)); //Point B

	plane3->setColor(glm::vec3(0, 1, 0));
	plane3->setSpecularity(false);
	sceneObjects.push_back(plane3);

	Plane* plane4 = new Plane(glm::vec3(50., -20, 10), //Point A
		glm::vec3(50., 40, 10), //Point B
		glm::vec3(50., 40, -260), //Point C
		glm::vec3(50., -20, -260)); //Point D

	plane4->setColor(glm::vec3(1, 0, 0));
	plane4->setSpecularity(false);
	sceneObjects.push_back(plane4);


	Plane* plane5 = new Plane(glm::vec3(-50., -20, -260), //Point A
		glm::vec3(50., -20, -260), //Point B
		glm::vec3(50., 40, -260), //Point C
		glm::vec3(-50., 40, -260)); //Point D

	plane5->setColor(glm::vec3(1, 0, 0));
	plane5->setSpecularity(false);
	sceneObjects.push_back(plane5);

	Plane* backWall = new Plane(glm::vec3(-50., -20, 10), //Point A
		glm::vec3(50., -20, 10), //Point B
		glm::vec3(50., 40, 10), //Point C
		glm::vec3(-50., 40, 10)); //Point D
	backWall->setColor(glm::vec3(0, 1, 0));
	backWall->setSpecularity(false);
	sceneObjects.push_back(backWall);


	Plane* mirror = new Plane(glm::vec3(-15., 0, -170), //Point A
		glm::vec3(20., 0, -170), //Point B
		glm::vec3(20., 30, -165), //Point C
		glm::vec3(-15., 30, -165)); //Point D
	mirror->setColor(glm::vec3(0, 0, 0));
	mirror->setReflectivity(true, 0.8);
	mirror->setSpecularity(false);
	sceneObjects.push_back(mirror);

	Cylinder* cylinder1 = new Cylinder(glm::vec3(-10.0, -20.0, -90.0), 4.0, 7.0);
	cylinder1->setColor(glm::vec3(0, 0.5, 0));   //Set colour to 
	//cylinder1->setReflectivity(true, 0.1);
	sceneObjects.push_back(cylinder1);

	Cylinder* cylinder2 = new Cylinder(glm::vec3(20.0, -20.0, -160.0), 2.0, 19.5);
	cylinder2->setColor(glm::vec3(0.3, 0.15, 0.15));   //Set colour to 
	//cylinder1->setReflectivity(true, 0.1);
	sceneObjects.push_back(cylinder2);

	Cylinder* cylinder3 = new Cylinder(glm::vec3(-15.5, 7.0, -100.0), 0.5, 23.0);
	cylinder3->setColor(glm::vec3(0.4, 0.3, 0.3));   //Set colour to 
	sceneObjects.push_back(cylinder3);



	Cone* cone1 = new Cone(glm::vec3(20.0, 3.0, -160.0), 6.0, 5.0);
	cone1->setColor(glm::vec3(0, 0.5, 0)); 
	sceneObjects.push_back(cone1);	

	Cone* cone2 = new Cone(glm::vec3(20.0, -3.0, -160.0), 9.0, 8.0);
	cone2->setColor(glm::vec3(0, 0.5, 0));
	sceneObjects.push_back(cone2);

	Cone* cone3 = new Cone(glm::vec3(20.0, -10.0, -160.0), 12.0, 10.0);
	cone3->setColor(glm::vec3(0, 0.5, 0));
	sceneObjects.push_back(cone3);

	Cone* cone4 = new Cone(glm::vec3(-15.0, -20.0, -150.0), 5.0, 12.0);
	cone4->setColor(glm::vec3(1, 0.5, 0));
	sceneObjects.push_back(cone4);
}


int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(20, 20);
    glutCreateWindow("Raytracing");

    glutDisplayFunc(display);
    initialize();

    glutMainLoop();
    return 0;
}

