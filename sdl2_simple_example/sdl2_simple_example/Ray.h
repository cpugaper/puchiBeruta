#ifndef RAY_H
#define RAY_H

#include <glm/glm.hpp>



class Ray {
public:
    glm::vec3 origin;  // Origen del rayo
    glm::vec3 direction;  // Direcci�n del rayo

    const float EPSILON = 1e-6f; 

    Ray(const glm::vec3& origin, const glm::vec3& direction)
        : origin(origin), direction(direction) {}

    // M�todo que calcula la intersecci�n del rayo con un plano (por ejemplo, el plano de un objeto)
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
        // Paso 1: Preparar los vectores del tri�ngulo
        glm::vec3 e1 = v1 - v0;
        glm::vec3 e2 = v2 - v0;
        glm::vec3 h = glm::cross(direction, e2);  // El vector h = d x e2

        float a = glm::dot(e1, h);  // Producto escalar de e1 y h

        // Si a es cercano a 0, el rayo es paralelo al tri�ngulo
        if (a > -EPSILON && a < EPSILON) {
            return false;
        }

        // Paso 2: Calcular el par�metro de la intersecci�n (baric�ntrico)
        float f = 1.0f / a;
        glm::vec3 s = origin - v0;
        float u = f * glm::dot(s, h);

        // Si u fuera fuera de los l�mites del tri�ngulo (0 <= u <= 1), no hay intersecci�n
        if (u < 0.0f || u > 1.0f) {
            return false;
        }

        glm::vec3 q = glm::cross(s, e1);
        float v = f * glm::dot(direction, q);

        // Si v fuera fuera de los l�mites del tri�ngulo (0 <= v <= 1 - u), no hay intersecci�n
        if (v < 0.0f || u + v > 1.0f) {
            return false;
        }

        // Paso 3: Calcular el valor de t (distancia del rayo al punto de intersecci�n)
        t = f * glm::dot(e2, q);

        // Si t es positivo, hay una intersecci�n
        return t > EPSILON;
    }

};

#endif // RAY_H