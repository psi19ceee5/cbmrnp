#ifndef PLANET_H
#define PLANET_H

#include "objects/drawable.h"
#include "image/image.h"

#include <memory>
#include <vector>
#include <stack>
#include <glm/vec3.hpp>


class Planet : public Drawable
{
public:
    Planet(std::string name = "UNNAMED PLANET",
            float radius = 1.0f,
            float distance = 10.0f,
            float spin = 0.f,
            float orbfreq = 0.f,
            float orbphase = 0.f,
            std::string textureLocation = ":/res/images/earth.bmp"
            );

    /**
     * @see Drawable::init()
     */
    virtual void init() override;

    /**
     * @see Drawable::recreate()
     */
    virtual void recreate() override;

    //virtual void initShader() override;

    /**
     * @see Drawable::draw(glm::mat4)
     */
    virtual void draw(glm::mat4 projection_matrix) const override;

    /**
     * @see Drawable::update(float, glm::mat4)
     */
    virtual void update(float elapsedTimeMs, glm::mat4 modelViewMatrix) override;

    void setCameraPosition(glm::vec3 camera);

    ~Planet();

    int triangles;


protected:

    std::vector<std::shared_ptr<Planet>> _children; /**< All children that move around this planet */

    float _radius;      /**< the radius of the planet */
    float _distance;    /**< the distance between this planet and its parent*/

    float _orbfreq;  /** the orbital frequency **/
    float _orbphase; /** the initial orbital phase **/

    float _localRotation;       /**< the current local roation */
    float _spin;  /**< the speed at which the planet spins */
    float _globalRotation;
    float _globalRotationSpeed;  /**< the speed at which the planet spins around parent*/

    std::string _textureLocation;

    std::vector<glm::vec3> positions;
    std::vector<unsigned int> indices;
    std::vector<glm::vec2> texcoords;
    std::vector<glm::vec3> normals;

    glm::vec3 _camera;

    GLuint textureID;


    glm::vec3 lightPos;


    /**
     * @see Drawable::createObject()
     */
    virtual void createObject() override;

    /**
     * @see Drawable::getVertexShader()
     */
    virtual std::string getVertexShader() const override;

    /**
     * @see Drawable::getFragmentShader()
     */
    virtual std::string getFragmentShader() const override;

    virtual GLuint loadTexture();


private:

    bool firstrender;


};


#endif // PLANET_H
