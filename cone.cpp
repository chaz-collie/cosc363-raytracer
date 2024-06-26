#include "cone.h"
#include <cmath>

//check for intersection
float Cone::intersect(glm::vec3 p0, glm::vec3 dir) {
    float yy = p0.y - centre.y;
    float tan = (radius/height) * (radius/height);

    float a = dir.x * dir.x + dir.z * dir.z - tan * dir.y * dir.y;
    float b = 2 * ((p0.x - centre.x) * dir.x + (p0.z - centre.z) * dir.z - tan * yy * dir.y);
    float c = (p0.x - centre.x) * (p0.x - centre.x) + (p0.z - centre.z) * (p0.z - centre.z) - tan * yy * yy;
    float d = sqrt(b * b - 4 * a * c);

    float x1 = (-b - d) / (2 * a);
    float x2 = (-b + d) / (2 * a);
    float y1 = p0.y + x1 * dir.y;
    float y2 = p0.y + x2 * dir.y;

    if (x1 > 0 && y1 >= centre.y && y1 <= centre.y + height) {
        return x1;
    }
   
    return false;
}

//normalise vector
glm::vec3 Cone::normal(glm::vec3 p) {
    glm::vec3 d = p - centre;
    float angle = radius / height;
    glm::vec3 cone(d.x, angle * sqrt(d.x * d.x + d.z * d.z), d.z);
    return glm::normalize(cone);
}