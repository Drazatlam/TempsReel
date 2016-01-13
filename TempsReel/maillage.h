#ifndef MAILLAGE_H
#define MAILLAGE_H
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <iostream>
#include <fstream>
#include <time.h>
#include <glm/vec3.hpp>
#include <vector>


class Maillage
{
    

    public:
		std::vector<glm::vec3> geom; //contient les points du maillage
    std::vector<int> topo; //contient des triplets d'indice pour chaque triangle du maillage
    std::vector<glm::vec3> normales; //contient les normales au point
    std::vector<int> normalIds;
        Maillage();
        Maillage(const Maillage& m);
        Maillage(std::vector<glm::vec3> geo, std::vector<int> top, std::vector<glm::vec3> norm) : geom(geo), topo(top), normales(norm) {}
        Maillage(std::vector<glm::vec3> geo, std::vector<int> top) : geom(geo), topo(top) {}



        // Utilitaires

        Maillage Rotation(const double matrice[3][3]);
        void Merge(Maillage figure2);
        void translate(const glm::vec3& t,glm::vec3 &min, glm::vec3 &max);
        void geometry(const glm::vec3 &center, const char* obj, glm::vec3 &min, glm::vec3 &max);

        ~Maillage();
        const std::vector<glm::vec3>& getGeom() const;
        void setGeom(const std::vector<glm::vec3> &value);
        const std::vector<int>& getTopo() const;
        void setTopo(const std::vector<int> &value);
        const std::vector<glm::vec3>& getNormales() const;
        void setNormales(const std::vector<glm::vec3> &value);
        const std::vector<int>& getNormalIds() const;
        void setNormalIds(const std::vector<int> &value);

		std::vector<float> points();
		std::vector<float> normals();

};

static const int taille = 500;
static const float coeffDetail = 0.03f;


#endif // MAILLAGE_H
