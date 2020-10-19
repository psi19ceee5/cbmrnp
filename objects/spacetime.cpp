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

    c_light_fraction = 0.8;
    omega = 4*M_PI_2/5.;
    R_N0 = 0.02;
    R_N1 = 0.02;
    gravConst = 1;
    density = 500;
    separation = 0.1;
    GM0 = gravConst*density*(4./3.)*2*M_PI_2*std::pow(R_N0, 3);
    GM1 = gravConst*density*(4./3.)*2*M_PI_2*std::pow(R_N1, 3);
    c_light = omega*separation/(2*c_light_fraction);
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
    glm::vec3 Ld(1.f);
    glUniform3fv(glGetUniformLocation(_program, "Ld"), 1, glm::value_ptr(Ld));
    float shininess = 2.f;
    glUniform1f(glGetUniformLocation(_program, "shininess"), shininess);
    glm::vec3 kd(1.f);
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
    positions.clear();
    indices.clear();
    vertex_normals.clear();
    texCoords.clear();

    scalefactor = 1.0;
    nside = 200;

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

    GLuint normal_buffer;
    glGenBuffers(1, &normal_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
    glBufferData(GL_ARRAY_BUFFER, vertex_normals.size() * 3 * sizeof(float), vertex_normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 0, 0);
    glEnableVertexAttribArray(1);

    GLuint tex_buffer;
    glGenBuffers(1,&tex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, tex_buffer);
    glBufferData(GL_ARRAY_BUFFER, texCoords.size()*sizeof (glm::vec2),texCoords.data(),GL_STATIC_DRAW);
    glVertexAttribPointer(2,2,GL_FLOAT,GL_TRUE,0,0);
    glEnableVertexAttribArray(2);

    // unbind vertex array object
    glBindVertexArray(0);

    // delete buffers (the data is stored in the vertex array object)
    glDeleteBuffers(1, &position_buffer);
    glDeleteBuffers(1, &index_buffer);
    glDeleteBuffers(1, &normal_buffer); // ok to delete?
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

            ypos = potential(xpos, zpos);
//            ypos = 0.1*sin(xpos/0.1 + (2*3.14/3)*time); // testfunction (plane wave)

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

    // calculation of normals
    glm::vec3 a_vec, b_vec, c_vec, current_pos;
    for(int j = 0; j < nside+1; ++j)
    {
        for(int i = 0; i < nside+1; ++i)
        {
            current_pos = positions[nindex(i, j)];

            if(j < nside && i < nside)
            {
                a_vec = positions[nindex(i+1, j)] - current_pos;
                b_vec = positions[nindex(i, j+1)] - current_pos;
            }

            if(j == nside && i < nside)
            {
                a_vec = positions[nindex(i, j-1)] - current_pos;
                b_vec = positions[nindex(i+1, j)] - current_pos;
            }

            if(j < nside && i == nside)
            {
                a_vec = positions[nindex(i, j+1)] - current_pos;
                b_vec = positions[nindex(i-1, j)] - current_pos;
            }

            if(j == nside && i == nside)
            {
                a_vec = positions[nindex(i-1, j)] - current_pos;
                b_vec = positions[nindex(i, j-1)] - current_pos;
            }

            c_vec = glm::cross(b_vec, a_vec);
            vertex_normals.push_back(glm::normalize(c_vec));
        }
    }
}

int
Spacetime::nindex(int i, int j)
{
    return i + j*(nside + 1);
}

glm::vec2
Spacetime::trajectory(float utime, int objectnr)
{
    glm::vec2 position = glm::vec2(sin(omega*utime + objectnr*2*M_PI_2), cos(omega*utime + objectnr*2*M_PI_2));

    if(objectnr == 0)
        position *= separation*(pow(R_N1, 3)/(pow(R_N0, 3) + pow(R_N1, 3)));
    else if(objectnr == 1)
        position *= separation*(pow(R_N0, 3)/(pow(R_N0, 3) + pow(R_N1, 3)));

    return position;
}

float
Spacetime::helperfunction(float delta_t, glm::vec2& rpos, int objectnr) // function that satisfies the retardation condition when equal to zero (depends on trajectory)
{
    float rho_N;
    if(objectnr == 0)
        rho_N = separation*(pow(R_N1, 3)/(pow(R_N0, 3) + pow(R_N1, 3)));
    else if(objectnr == 1)
        rho_N = separation*(pow(R_N0, 3)/(pow(R_N0, 3) + pow(R_N1, 3)));

    float phi = omega*(time - delta_t) + objectnr*2*M_PI_2;

    return c_light_fraction*sqrt(pow(rpos.x,2) + pow(rpos.y,2) + rho_N*rho_N - 2*rho_N*(rpos.x*sin(phi) + rpos.y*cos(phi))) - rho_N*omega*delta_t;
}

float
Spacetime::ddt_helpfunc(float delta_t, glm::vec2& rpos, int objectnr) // derivative (d/d(delta_t) of helperfunction
{
    float rho_N;
    if(objectnr == 0)
        rho_N = separation*(pow(R_N1, 3)/(pow(R_N0, 3) + pow(R_N1, 3)));
    else if(objectnr == 1)
        rho_N = separation*(pow(R_N0, 3)/(pow(R_N0, 3) + pow(R_N1, 3)));

    float phi = omega*(time - delta_t) + objectnr*2*M_PI_2;
    float result = c_light_fraction*rho_N*omega*(rpos.x*cos(phi) - rpos.y*sin(phi));
    result /= sqrt(rpos.x*rpos.x +  rpos.y*rpos.y + rho_N*rho_N - 2*rho_N*(rpos.x*sin(phi) + rpos.y*cos(phi)));
    result -= rho_N*omega;

    return result;
}

float
Spacetime::retardedDistance(glm::vec2& rpos, glm::vec2& r0, int objectnr, int iterations)
{
    float delta_t_start = glm::length(rpos - r0)/c_light;
    float delta_t = delta_t_start;

    float a, b;

    int n = 0;
    do
    {
        delta_t = delta_t_start - n*.1;
        for(int i = 0; i < iterations; ++i)
        {
            a = ddt_helpfunc(delta_t, rpos, objectnr);
            b = helperfunction(delta_t, rpos, objectnr) - a*delta_t;
            delta_t = -1*b/a;
        }
        ++n;
    }
    while(abs(helperfunction(delta_t, rpos, objectnr)) > 0.01);

    if(abs(helperfunction(delta_t, rpos, objectnr)) > 0.0001)
        std::cout << helperfunction(delta_t, rpos, objectnr) << std::endl;

    glm::vec2 r0_ret = glm::vec2(trajectory(time - delta_t, objectnr));
    return glm::length(rpos - r0_ret);
}

float
Spacetime::potential(float xpos, float zpos)
{
    glm::vec2 rpos = glm::vec2(xpos, zpos);

    glm::vec2 r0 = trajectory(time, 0);
    glm::vec2 r1 = trajectory(time, 1);

    float dist0_ret = retardedDistance(rpos, r0, 0, 5);
    float dist1_ret = retardedDistance(rpos, r1, 1, 5);

    float potential0;
    float potential1;

    if(dist0_ret < R_N0)
        potential0 = 0.5*GM0*std::pow(dist0_ret, 2)/pow(R_N0, 3) - 1.5*GM0/R_N0;
    else
        potential0 = -1*GM0/dist0_ret;

    if(dist1_ret < R_N1)
        potential1 = 0.5*GM1*std::pow(dist1_ret, 2)/pow(R_N1, 3) - 1.5*GM1/R_N1;
    else
        potential1 = -1*GM1/dist1_ret;

    float potential = potential0 + potential1;

    return potential;
}
