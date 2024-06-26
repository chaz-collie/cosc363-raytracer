//cone header file
#ifndef H_CONE
#define H_CONE

#include <glm/glm.hpp>
#include "SceneObject.h"

class Cone : public SceneObject
{
private:
	glm::vec3 centre = glm::vec3(0);
	float height = 1;
    float radius = 1;

public:	
	Cone() {};
	Cone(glm::vec3 c, float r, float h) : centre(c), radius(r), height(h) {}
    float intersect(glm::vec3 p0, glm::vec3 dir);
    glm::vec3 normal(glm::vec3 p);
};

#endif