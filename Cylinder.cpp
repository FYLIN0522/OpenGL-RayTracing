/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The cylinder class
*  This is a subclass of Object, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/

#include "Cylinder.h"
#include <math.h>

/**
* cylinder's intersection method.  The input is a ray. 
*/
float Cylinder::intersect(glm::vec3 p0, glm::vec3 dir)
{
    
    float a = (dir.x * dir.x) + (dir.z * dir.z);
    float b = 2 * (dir.x * (p0.x - center.x) + dir.z * (p0.z - center.z));
    float c = ((p0.x - center.x) * (p0.x - center.x) + 
                (p0.z - center.z) * (p0.z - center.z) - 
                (radius * radius));

    float delta = b * b - 4 * a * c;
    if (delta < 0.001) return -1.0;    //includes zero and negative values

    float t1 = (-b + sqrt(delta)) / (2 * a);
    float t2 = (-b - sqrt(delta)) / (2 * a);
    if (t1 < 0.001) t1 = -1;
    if (t2 < 0.001) t2 = -1;

    float t_near, t_far;
    if (t1 > t2) {
        t_near = t2;
        t_far = t1;
    } else {
        t_near = t1;
        t_far = t2;
    }

    glm::vec3 p1 = p0 + t_near * dir;
    glm::vec3 p2 = p0 + t_far * dir;
 
    float cap = (height + center.y - p1.y) / dir.y;

    if (t_near > 0 && (p1.y > height + center.y && p2.y < height + center.y && p2.y >= center.y)) {
        return cap;
    }

    else if (t_near > 0 && (p1.y <= height + center.y && p1.y >= center.y)) {
        return t_near;
    }

    else if (t_far > 0 && (p2.y <= height + center.y && p2.y >= center.y)) {
        return t_far;
    }

    else {
        return -1;
    }
}   

/**
* Returns the unit normal vector at a given point.
* Assumption: The input point p lies on the sphere.
*/
 glm::vec3 Cylinder::normal(glm::vec3 p)
 {
     glm::vec3 n;
     if (p.y >= center.y + height) {
         n = glm::vec3(0, 1, 0);
         return n;
     }

     n = glm::vec3((p.x - center.x)/radius, 0, (p.z - center.z)/radius);
     return n;
 }
