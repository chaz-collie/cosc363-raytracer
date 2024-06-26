#include "cylinder.h"
#include <math.h>

//check for intersection
float Cylinder::intersect(glm::vec3 p0, glm::vec3 dir) {
    float a = dir.x * dir.x + dir.z * dir.z;
    float b = 2 * (dir.x * (p0.x - centre.x) + dir.z * (p0.z - centre.z));
    float c = (p0.x - centre.x) * (p0.x - centre.x) + (p0.z - centre.z) * (p0.z - centre.z) - radius * radius;
    float delta = sqrt(b * b - 4 * a * c);

    float x1 = (-b - delta) / (2 * a);
    float x2 = (-b + delta) / (2 * a);
    float y1 = p0.y + x1 * dir.y;
    float y2 = p0.y + x2 * dir.y;
    float cylinderUnder = centre.y;
    float cylinderCap = centre.y + height;

    //checks if within the height bounds
    if (x1 > 0 && y1 >= cylinderUnder && y1 <= cylinderCap){ 
        return x1;
    }

    //makes the cap
    if (x2 > 0 && y2 >= cylinderUnder && y2 <= cylinderCap)
    {
        return (centre.y + height - p0.y) / dir.y;
    }

    return false;
}


//normalise vector
glm::vec3 Cylinder::normal(glm::vec3 p) {
    if (p.y == centre.y + height)
    {
        return glm::vec3(0, 1, 0);
    }
    glm::vec3 cylinder(p.x - centre.x, 0, p.z - centre.z);
    return glm::normalize(cylinder);
}