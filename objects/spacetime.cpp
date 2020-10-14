#include <GL/glew.h>

#include "spacetime.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <iostream>
#include <stack>
#include <cmath>

#include <QFile>
#include <QTextStream>

#include <QKeyEvent>

#include "glbase/gltool.hpp"
#include "image/image.h"
#include "glbase/texload.hpp"

#include "gui/config.h"

Spacetime::Spacetime(std::string name, std::string textureLocation): Drawable(name)
{
    _textureLocation=textureLocation;
    time = 0.f;
}

void
Spacetime::init()
{
    Drawable::init();

    loadFBO();

    loadTexture();
}

void
Spacetime::draw(glm::mat4 projection_matrix) const
{

    // Load program
    glUseProgram(_program);

    // bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,textureID);

    // bind vertex array object
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

    // unbind vertex array object
    glBindVertexArray(0);

    // check for errors
    VERIFY(CG::checkError());

}

void
Spacetime::update(float elapsedTimeMs, glm::mat4 modelViewMatrix)
{
    _modelViewMatrix = modelViewMatrix;
    time += elapsedTimeMs/1000.;
}

std::string
Spacetime::getVertexShader() const
{
    return Drawable::loadShaderFile(":/shader/spacetime.vs.glsl");
}

std::string
Spacetime::getFragmentShader() const
{
    return Drawable::loadShaderFile(":/shader/spacetime.fs.glsl");
}

void
Spacetime::createObject()
{
    texCoords.clear();
    positions.clear();
    indices.clear();

    scalefactor = 1.0;
    nside = 300;

    calcPositions();

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

    GLuint tex_buffer;
    glGenBuffers(1,&tex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, tex_buffer);
    glBufferData(GL_ARRAY_BUFFER, texCoords.size()*sizeof (glm::vec2),texCoords.data(),GL_STATIC_DRAW);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_TRUE,0,0);
    glEnableVertexAttribArray(1);

    // unbind vertex array object
    glBindVertexArray(0);

    // delete buffers (the data is stored in the vertex array object)
    glDeleteBuffers(1, &position_buffer);
    glDeleteBuffers(1, &index_buffer);
    glDeleteBuffers(1,&tex_buffer);

    // check for errors
    VERIFY(CG::checkError());
}

GLuint
Spacetime::loadTexture()
{
    glGenTextures(1,&textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    Image image(_textureLocation);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.getWidth(),image.getHeight(),0,GL_RGBA, GL_UNSIGNED_BYTE, image.getData());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glGenerateMipmap(GL_TEXTURE_2D);

    VERIFY(CG::checkError());

    return textureID;
}


void
Spacetime::loadFBO()
{
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

    glGenFramebuffers(1, &depthMapFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void
Spacetime::calcPositions()
{
    float xpos, ypos, zpos;
    float xtex, ytex;

    for(int j = 0; j < nside+1; ++j)
    {
        zpos = -1 + 2*float(j)/float(nside);
        ytex = float(j)/float(nside);

        for(int i = 0; i < nside+1; ++i)
        {
            xpos = -1 + 2*float(i)/float(nside);
            xtex = float(i)/float(nside);

            ypos = 0.1*sin(xpos/0.1 + (2*3.14/3)*time);
            positions.push_back(glm::vec3(xpos, ypos, zpos));
            texCoords.push_back(glm::vec2(xtex, ytex));

            if(j < nside && i < nside) // define triangles on grid
            {
                indices.push_back(nindex(i  ,j  ));
                indices.push_back(nindex(i  ,j+1));
                indices.push_back(nindex(i+1,j  ));

                indices.push_back(nindex(i+1,j  ));
                indices.push_back(nindex(i  ,j+1));
                indices.push_back(nindex(i+1,j+1));
            }
        }
    }

    for(auto & pos : positions) pos *= scalefactor;
}

int
Spacetime::nindex(int i, int j)
{
    return i + j*(nside + 1);
}
