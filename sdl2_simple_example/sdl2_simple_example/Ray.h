#ifndef RAY_H
#define RAY_H

#include <glm/glm.hpp>
#include <SDL2/SDL_events.h>
#include <cereal/types/vector.hpp>
#include <cereal/types/base_class.hpp> 
#include <cereal/archives/json.hpp>


class Ray {
public:
    glm::vec3 origin;
    glm::vec3 direction;

    Ray(const glm::vec3& origin, const glm::vec3& direction)
        : origin(origin), direction(direction) {}

    // Intersección con un plano (usado para chequear colisiones)
    bool intersectsPlane(const glm::vec3& planeNormal, const glm::vec3& planePoint, float& t) const {
        float denom = glm::dot(planeNormal, direction);
        if (abs(denom) > 0.0001f) {
            glm::vec3 p0l0 = planePoint - origin;
            t = glm::dot(p0l0, planeNormal) / denom;
            return (t >= 0);
        }
        return false;
    }
};
#endif