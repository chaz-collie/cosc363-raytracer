/*==================================================================================
* COSC 363  Computer Graphics
* Department of Computer Science and Software Engineering, University of Canterbury.
*
* A basic ray tracer
* See Lab07.pdf   for details.
*===================================================================================
*/
#include "TextureBMP.h"
#include <iostream>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include "Sphere.h"
#include "SceneObject.h"
#include "Ray.h"
#include <GL/freeglut.h>
#include "Plane.h"
#include "cylinder.h"
#include "cone.h"

using namespace std;

TextureBMP texture;
const float EDIST = 40.0;
const int NUMDIV = 500;
const int MAX_STEPS = 5;
const float XMIN = -10.0;
const float XMAX = 10.0;
const float YMIN = -10.0;
const float YMAX = 10.0;

vector<SceneObject*> sceneObjects;

//---The most important function in a ray tracer! ---------------------------------- 
//   Computes the colour value obtained by tracing a ray and finding its 
//     closest point of intersection with objects in the scene.
//----------------------------------------------------------------------------------

glm::vec3 trace(Ray ray, int step)
{
	glm::vec3 backgroundCol(0);
	glm::vec3 lightPos1(-10.0, 29.0, -40.0);
    glm::vec3 lightPos2(10.0, 29.0, -30.0);
    glm::vec3 lightPositions[2] = {lightPos1, lightPos2};
    glm::vec3 color(0);
    glm::vec3 totalLightColor(0);
	SceneObject* obj;

    ray.closestPt(sceneObjects);//Compare the ray with all objects in the scene

    if(ray.index == -1) return backgroundCol;//no intersection
	obj = sceneObjects[ray.index];//object on which the closest point of intersection is found

    if (ray.index == 0) //for the icecream texture on sphere
    {
        glm::vec3 normal = obj->normal(ray.hit);
        float u = 0.5 + ((atan2(normal.z,normal.x)) / (2*M_PI));
        float v = 0.5 + ((asin(normal.y)) / (M_PI));
        color = texture.getColorAt(u, v);
        obj->setColor(color);
    }

	if (ray.index == 2) //for the checkerboard pattern on the bottom plane
    {
        int checkSize = 5;
        int ix = (ray.hit.x + 60) / checkSize;
        int iz = (ray.hit.z) / checkSize;
        int k = (ix + iz) % 2;
        if (k == 0) 
        {
            color = glm::vec3(0.9, 0.9, 0.98);
        }
        else
        {
            color = glm::vec3(0.94, 0.98, 0.9);    
        } 
        obj->setColor(color);
    }

    //loop to go through multiple light sources so shadows come from both lights
    for (int i = 0; i < 2; ++i){
        glm::vec3 lightPos = lightPositions[i];
        glm::vec3 lightVec = lightPos - ray.hit;
        float lightDist = glm::length(lightVec);
        Ray shadowRay(ray.hit, lightVec);
        shadowRay.closestPt(sceneObjects);

        glm::vec3 lightColor = obj->lighting(lightPos, -ray.dir, ray.hit);

        if ((shadowRay.index > -1) && (shadowRay.dist < lightDist)) 
        {
            SceneObject* hitObject = sceneObjects[shadowRay.index];
            if (hitObject->isRefractive() || hitObject->isTransparent()) {
                lightColor = 0.8f * obj->getColor() + 0.1f * hitObject->getColor();
            } else {
                lightColor = 0.2f * obj->getColor();
            }
        }
        totalLightColor += lightColor;
    }

    color = totalLightColor / 2.0f;

    //reflective ray calc
	if (obj->isReflective() && step < MAX_STEPS) {
        float a = obj->getReflectionCoeff();
        glm::vec3 normVector = obj->normal(ray.hit);
        glm::vec3 reflecDir = glm::reflect(ray.dir, normVector);
        Ray reflecRay(ray.hit, reflecDir);
        glm::vec3 reflectedColor = trace(reflecRay, step + 1);
        color = color + (a * reflectedColor);
    }

    //transparent ray calc
    if (obj->isTransparent() and step < MAX_STEPS)
    {
        float a = obj->getTransparencyCoeff();
        Ray reflecRay(ray.hit, ray.dir);
        reflecRay.closestPt(sceneObjects);
        Ray secondRay(reflecRay.hit, ray.dir);
        glm::vec3 reflectedColor = trace(secondRay, step + 1);
        color = ((1 - a) * color) + (a * reflectedColor);
    }

    //refractive ray calc
    if (obj->isRefractive() && step < MAX_STEPS) 
    {
        glm::vec3 normVector = obj->normal(ray.hit);
        glm::vec3 refrDir = glm::refract(ray.dir, normVector, 1 / 1.01f); //entering object
        Ray refrRay(ray.hit, refrDir);
        refrRay.closestPt(sceneObjects);
        
        if (refrRay.index == -1) {
            return backgroundCol;
        } else { 
            glm::vec3 newNorm = sceneObjects[refrRay.index]->normal(refrRay.hit);
            glm::vec3 exitRefrDir = glm::refract(refrDir, -newNorm, 1.01f); //exiting object
            Ray exitRay(refrRay.hit, exitRefrDir);
            glm::vec3 refractedColor = trace(exitRay, step + 1);
            color += refractedColor;
        }
    }


	return color;
}

glm::vec3 antiAliasing(float xp, float yp, float cellX, float cellY, glm::vec3 eye) 
{
    glm::vec3 samples[4];
    glm::vec3 sampleColors[4];
    glm::vec3 color(0.0);

    //initial 2x2 sampling
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            float xOffset = (i + 0.5f) * 0.5f * cellX;
            float yOffset = (j + 0.5f) * 0.5f * cellY;
            glm::vec3 dir(xp + xOffset, yp + yOffset, -EDIST);
            Ray ray = Ray(eye, dir);
            color += trace(ray, 1);
        }
    }
    //calc color variance
    glm::vec3 avgCol = (sampleColors[0] + sampleColors[1] + sampleColors[2] + sampleColors[3]) * 0.25f;
    glm::vec3 var = ((sampleColors[0] - avgCol) * (sampleColors[0] - avgCol) + 
                      (sampleColors[1] - avgCol) * (sampleColors[1] - avgCol) + 
                      (sampleColors[2] - avgCol) * (sampleColors[2] - avgCol) + 
                      (sampleColors[3] - avgCol) * (sampleColors[3] - avgCol)) * 0.25f;
    float varianceMagnitude = glm::length(var);

    //if var is high, do more sampling
    if (varianceMagnitude > 0.05f) {
        color = glm::vec3(0.0f);
        for (int i = 0; i < 4; ++i) {
            float xOffset = (i % 2) * 0.5f * cellX + 0.25f * cellX;
            float yOffset = (i / 2) * 0.5f * cellY + 0.25f * cellY;
            glm::vec3 dir(xp + xOffset, yp + yOffset, -EDIST);
            Ray ray = Ray(eye, dir);
            color += trace(ray, 1);
        }
        color *= 0.25f;
    } else { //if var isnt high take the var, save on time
        color = avgCol;
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
    glm::vec3 eye(0., 0., 0.);

    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glBegin(GL_QUADS);  //Each cell is a tiny quad.

    for (int i = 0; i < NUMDIV; i++)    //Scan every cell of the image plane
    {
        xp = XMIN + i * cellX;
        for (int j = 0; j < NUMDIV; j++)
        {
            yp = YMIN + j * cellY;

            glm::vec3 col = antiAliasing(xp, yp, cellX, cellY, eye); // do anti aliasing

            glColor3f(col.r, col.g, col.b);
            glVertex2f(xp, yp);             //Draw each cell with its color value
            glVertex2f(xp + cellX, yp);
            glVertex2f(xp + cellX, yp + cellY);
            glVertex2f(xp, yp + cellY);
        }
    }

    glEnd();
    glFlush();
}

void drawBox()
{
    //bottom plane
    Plane *plane1 = new Plane(glm::vec3(-25., -15, 40), glm::vec3(25., -15, 40),
                              glm::vec3(25., -15, -200), glm::vec3(-25., -15, -200));

    sceneObjects.push_back(plane1);

    //top plane
    Plane *plane2 = new Plane(glm::vec3(-25., 30, 40), glm::vec3(-25., 30, -200),
                            glm::vec3(25., 30, -200), glm::vec3(25., 30, 40));
    plane2->setColor(glm::vec3(1, 1, 0.87));
    plane2->setSpecularity(false);
    sceneObjects.push_back(plane2);

    //left plane
    Plane *plane3 = new Plane(glm::vec3(-25., -15, 40), glm::vec3(-25., -15, -200),
                              glm::vec3(-25., 30, -200), glm::vec3(-25., 30, 40));
    plane3->setColor(glm::vec3(1, 0.81, 0.86));
    plane3->setSpecularity(false);
    sceneObjects.push_back(plane3);

    //right plane
    Plane *plane4 = new Plane(glm::vec3(25., -15, 40), glm::vec3(25., 30, 40),
                            glm::vec3(25., 30, -200), glm::vec3(25., -15, -200));
    plane4->setColor(glm::vec3(0.67, 0.84, 0.9));
    plane4->setSpecularity(false);
    sceneObjects.push_back(plane4);

    //back plane
    Plane *plane5 = new Plane(glm::vec3(-25, -15, -200), glm::vec3(25., -15, -200),
                                glm::vec3(25., 30, -200), glm::vec3(-25, 30, -200));
    plane5->setColor(glm::vec3(0, 0, 0));
    plane5->setSpecularity(false);
    plane5->setReflectivity(true);
    sceneObjects.push_back(plane5);

    //front Plane
    Plane *plane6 = new Plane(glm::vec3(-25, -15, 40), glm::vec3(25., -15, 40),
                              glm::vec3(25., 30, 40), glm::vec3(-25, 30, -40));
    plane6->setColor(glm::vec3(0, 0, 0));
    plane6->setSpecularity(false);
    plane6->setReflectivity(true);
    sceneObjects.push_back(plane6);
}

//function to make a transparent sphere
void transparentSphere()
{
    Sphere *sphere2 = new Sphere(glm::vec3(-8.0, 0.0, -80.0), 3.0);
    sphere2->setColor(glm::vec3(0.67, 0.9, 0.67));
    sphere2->setShininess(5);
	sphere2->setTransparency(true, 0.8);
	sceneObjects.push_back(sphere2);    
}

//function to make a sphere with a refractive surface
void refractiveSphere()
{
    Sphere *sphere3 = new Sphere(glm::vec3(0.0, -9.0, -70.0), 4.0);
    sphere3->setColor(glm::vec3(0, 0, 0));
	sphere3->setRefractivity(true);
	sceneObjects.push_back(sphere3);
}


//function which makes icecream from a cone and textured sphere
void icecream()
{

    texture = TextureBMP("icecream.bmp");
    Sphere *sphere4 = new Sphere(glm::vec3(17.0, -10.0, -80.0), 3.0);
    sphere4->setColor(glm::vec3(0, 0, 0));
	sceneObjects.push_back(sphere4);

    Cone *cone = new Cone(glm::vec3(10.0, -10.0, -90.0), 3.0, 10.0);
    cone->setColor(glm::vec3(0.75,0.6,0.4));
    sceneObjects.push_back(cone);

}

//function for a capped cylinder
void cappedCylinder()
{
    Cylinder *cylinder = new Cylinder(glm::vec3(-17.0, -15.0, -90.0), 3.0, 5.0);
    cylinder->setColor(glm::vec3(0.67, 0.84, 0.9));
    sceneObjects.push_back(cylinder);
}

//---This function initializes the scene ------------------------------------------- 
//   Specifically, it creates scene objects (spheres, planes, cones, cylinders etc)
//     and add them to the list of scene objects.
//   It also initializes the OpenGL 2D orthographc projection matrix for drawing the
//     the ray traced image.
//----------------------------------------------------------------------------------
void initialize()
{
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(XMIN, XMAX, YMIN, YMAX);
    glClearColor(0, 0, 0, 1);

    //call all the objects
    icecream();
    drawBox();
    transparentSphere();
    refractiveSphere();
    cappedCylinder();
}

int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB );
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(20, 20);
    glutCreateWindow("Raytracing Assignment");
    glutDisplayFunc(display);
    initialize();
    glutMainLoop();
    return 0;
}