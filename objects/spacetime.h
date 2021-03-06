#ifndef SPACETIME_H
#define SPACETIME_H

#include "objects/drawable.h"
#include "image/image.h"
#include <memory>
#include <vector>
#include <stack>
#include <glm/vec3.hpp>

class Spacetime : public Drawable
{
public:
    Spacetime(std::string name = "SPACETIME", std::string textureLocation = ":/res/images/gridlines.png");

    /**
     * @see Drawable::init()
     */
    virtual void init() override;

    /**
     * @see Drawable::draw(glm::mat4)
     */
    virtual void draw(glm::mat4 projection_matrix) const override;

    /**
     * @see Drawable::update(float, glm::mat4)
     */
    virtual void update(float elapsedTimeMs, glm::mat4 modelViewMatrix) override;

protected:


    /**
     * @see Drawable::getVertexShader()
     */
    virtual std::string getVertexShader() const override;

    /**
     * @see Drawable::getFragmentShader()
     */
    virtual std::string getFragmentShader() const override;

    /**
     * @see Drawable::createObject()
     */
    virtual void createObject() override;

    /**
     * @see Drawable::loadTexture()
     */
    virtual GLuint loadTexture() override;

    // everything needed for shadows
    GLuint depthMapFBO;
    GLuint depthMap;

    void loadFBO();

    void calcPositions();
    int nindex(int, int);

    glm::vec2 trajectory(float, int);
    float helperfunction(float, glm::vec2&, int);
    float ddt_helpfunc(float delta_t, glm::vec2&, int objectnr);
    float retardedDistance(glm::vec2&, glm::vec2&, int, int);
    float potential(float, float);

    std::vector<glm::vec3> positions;
    std::vector<unsigned int> indices;
    std::vector<glm::vec3> vertex_normals;
    std::vector<glm::vec2> texCoords;

    int nside;
    float scalefactor;
    float time;

    // physical parameters
    float
    c_light,
    c_light_fraction,
    omega,
    R_N0,
    R_N1,
    gravConst,
    density,
    separation,
    GM0,
    GM1;
};

#endif // SPACETIME_H
