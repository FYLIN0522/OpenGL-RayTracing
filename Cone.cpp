/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The cylinder class
*  This is a subclass of Object, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/

#include "Cone.h"
#include <math.h>
#include <cmath>


/**
* cylinder's intersection method.  The input is a ray.
*/
float Cone::intersect(glm::vec3 p0, glm::vec3 dir)
{
    //a = dx ^ 2 + dz ^ 2 - (R ^ 2 / h ^ 2) * dy ^ 2;
    //b = 2 * dx * (x0 - xc) + 2 * dz * (z0 - zc) + 2 * (R ^ 2 / h ^ 2) * dy * (yc - y0 + h);
    //c = (x0 - xc) ^ 2 + (z0 - zc) ^ 2 - (R ^ 2 / h ^ 2) * (yc - y0 + h) ^ 2;

    float a = (dir.x * dir.x) + (dir.z * dir.z) - (radius / height) * (radius / height) * dir.y * dir.y;

    float b = (2 * dir.x * (p0.x - center.x)) +
                (2 * dir.z * (p0.z - center.z)) +
                (2 * (radius / height) * (radius / height) * dir.y * (center.y - p0.y + height));

    float c = (p0.x - center.x) * (p0.x - center.x) +
                (p0.z - center.z) * (p0.z - center.z) -
                (radius / height) * (radius / height) * (center.y - p0.y + height) * (center.y - p0.y + height);
    

    float delta = b * b - 4 * (a * c);
    if (delta < 0.001) return -1.0;    //includes zero and negative values

    float t1 = (-b + sqrt(delta)) / (2 * a);
    float t2 = (-b - sqrt(delta)) / (2 * a);


    float t_max, t_min;
    if (t1 > t2) {
        t_min = t2;
        t_max = t1;
    }
    else {
        t_min = t1;
        t_max = t2;
    }

    glm::vec3 p = p0 + t_min * dir;

    if (t_min > 0 && (p.y <= height + center.y && p.y >= center.y)) {
        return t_min;
    } else {
        return -1;
    }
}


/**
* Returns the unit normal vector at a given point.
* Assumption: The input point p lies on the sphere.
*/
glm::vec3 Cone::normal(glm::vec3 p)
{
    float theta = atan(radius / height);
    float alpha = atan((p.x - center.x) / (p.z - center.z));
    glm::vec3 n (sin(alpha) * cos(theta), sin(theta), cos(alpha) * cos(theta));
    return n;    
}
