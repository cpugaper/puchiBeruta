#ifndef RAY_H
#define RAY_H

#include <glm/glm.hpp>

class Ray {
public:
    glm::vec3 origin;
    glm::vec3 direction;

    const float EPSILON = 1e-6f; 

    Ray(const glm::vec3& origin, const glm::vec3& direction)
        : origin(origin), direction(direction) {}

    // Calculate the intersection of the ray with a plane.
    bool intersects(const glm::vec3& planePoint, const glm::vec3& planeNormal, float& t) {
        float denom = glm::dot(planeNormal, direction);
        if (abs(denom) > 1e-6) {
            glm::vec3 p0l0 = planePoint - origin;
            t = glm::dot(p0l0, planeNormal) / denom;
            return (t >= 0);
        }
        return false;
    }

    bool intersectsTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, float& t) {
        glm::vec3 e1 = v1 - v0;
        glm::vec3 e2 = v2 - v0;
        glm::vec3 h = glm::cross(direction, e2);

        float a = glm::dot(e1, h);

        // If a is close to 0, the ray is parallel to the triangle
        if (a > -EPSILON && a < EPSILON) {
            return false;
        }

        // Calculate the intersection parameter (barycentric)
        float f = 1.0f / a;
        glm::vec3 s = origin - v0;
        float u = f * glm::dot(s, h);

        if (u < 0.0f || u > 1.0f) {
            return false;
        }

        glm::vec3 q = glm::cross(s, e1);
        float v = f * glm::dot(direction, q);

        if (v < 0.0f || u + v > 1.0f) {
            return false;
        }

        // Calculate the value of t (distance from the ray to the intersection point).
        t = f * glm::dot(e2, q);

        return t > EPSILON;
    }
};

#endif // RAY_H