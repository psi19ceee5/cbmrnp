#include <GL/glew.h>

#include "spacetime.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <iostream>
#include <stack>

#include <QFile>
#include <QTextStream>

#include <QKeyEvent>

#include "glbase/gltool.hpp"
#include "image/image.h"
#include "glbase/texload.hpp"

#include "gui/config.h"

Spacetime::Spacetime(std::string name): Drawable(name) {};

void Spacetime::init()
{

    Drawable::init();

    loadFBO();

    //    loadTexture();
}

void Spacetime::draw(glm::mat4 projection_matrix) const
{

    // Load program
    glUseProgram(_program);

    // bin vertex array object
    glBindVertexArray(_vertexArrayObject);

    // set parameter
    glUniformMatrix4fv(glGetUniformLocation(_program, "projection_matrix"), 1, GL_FALSE, glm::value_ptr(projection_matrix));
    glUniformMatrix4fv(glGetUniformLocation(_program, "modelview_matrix"), 1, GL_FALSE, glm::value_ptr(_modelViewMatrix));
    glUniform1i(glGetUniformLocation(_program, "texture"), 0);


    VERIFY(CG::checkError());

    //lighting values
    glm::vec3 La(0.6f);
    glUniform3fv(glGetUniformLocation(_program, "La"), 1, glm::value_ptr(La));
    glm::vec3 Ls(1.0, 1.0, 1.0);
    glUniform3fv(glGetUniformLocation(_program, "Ls"), 1, glm::value_ptr(Ls));
    glm::vec3 Ld(0.6f);
    glUniform3fv(glGetUniformLocation(_program, "Ld"), 1, glm::value_ptr(Ld));
    float shininess = 2.f;
    glUniform1f(glGetUniformLocation(_program, "shininess"), shininess);
    glm::vec3 kd(0.9f);
    glUniform3fv(glGetUniformLocation(_program, "kd"), 1, glm::value_ptr(kd));
    glm::vec3 ks(.0f);
    glUniform3fv(glGetUniformLocation(_program, "ks"), 1, glm::value_ptr(ks));
    glm::vec3 ka(0.5f);
    glUniform3fv(glGetUniformLocation(_program, "ka"), 1, glm::value_ptr(ka));


    // call draw
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    // unbin vertex array object
    glBindVertexArray(0);

    // check for errors
    VERIFY(CG::checkError());

}

void Spacetime::update(float elapsedTimeMs, glm::mat4 modelViewMatrix)
{
    _modelViewMatrix = modelViewMatrix;
}

void Spacetime::createObject()
{
    std::vector<glm::vec2> texCoords;
    positions.clear();
    indices.clear();


    float factor = 100;

    positions.push_back(glm::vec3(1,0,1)*factor);    //0
    positions.push_back(glm::vec3(-1,0,1)*factor);   //1
    positions.push_back(glm::vec3(1,0,-1)*factor);   //2
    positions.push_back(glm::vec3(-1,0,-1)*factor);  //3

    indices.push_back(0);
    indices.push_back(3);
    indices.push_back(1);

    indices.push_back(0);
    indices.push_back(2);
    indices.push_back(3);

    glm::vec3 stcolor = glm::vec3(0,0.5,1);
    
    
    // Set up a vertex array object for the geometry
    if(_vertexArrayObject == 0)
      glGenVertexArrays(1, &_vertexArrayObject);
    glBindVertexArray(_vertexArrayObject);


    // fill vertex array object with data
    GLuint position_buffer;
    glGenBuffers(1, &position_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * 3 * sizeof(float), positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);


    GLuint index_buffer;
    glGenBuffers(1, &index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // unbind vertex array object
    glBindVertexArray(0);

    // delete buffers (the data is stored in the vertex array object)
    glDeleteBuffers(1, &position_buffer);
    glDeleteBuffers(1, &index_buffer);

    // check for errors
    VERIFY(CG::checkError());
}


void Spacetime::loadFBO()
{
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

    glGenFramebuffers(1, &depthMapFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

std::string Spacetime::getVertexShader() const
{
    return Drawable::loadShaderFile(":/shader/spacetime.vs.glsl");
}

std::string Spacetime::getFragmentShader() const
{
    return Drawable::loadShaderFile(":/shader/spacetime.fs.glsl");
}
