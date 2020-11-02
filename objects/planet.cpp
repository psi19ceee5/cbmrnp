#include <GL/glew.h>
#include "planet.h"


#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
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


Planet::Planet(std::string name,
               float radius,
               float distance,
               float spin,
               float orbfreq,
               float orbphase,
               std::string textureLocation):
    Drawable(name),
    _radius(radius),
    _distance(distance),
    _orbfreq(orbfreq),
    _orbphase(orbphase),
    _localRotation(0),
    _spin(spin),
    _globalRotation(0)
{
    _spin = spin; // for local rotation:one step equals one hour
    _orbfreq = orbfreq; // for global rotation

    _textureLocation = textureLocation;
}

void Planet::init()
{
    Drawable::init();

    lightPos = glm::vec3(0.f, 0.f, 0.f);

    //load texture
    loadTexture();
}



void Planet::recreate()
{

    createObject();
}



void Planet::draw(glm::mat4 projection_matrix) const
{

    if(_program == 0){
        std::cerr << "Planet" << _name << "not initialized. Call init() first." << std::endl;
        return;
    }

    // Load program
    glUseProgram(_program);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,textureID);

    VERIFY(CG::checkError());

    // bin vertex array object
    glBindVertexArray(_vertexArrayObject);

    // set parameter
    glUniformMatrix4fv(glGetUniformLocation(_program, "projection_matrix"), 1, GL_FALSE, glm::value_ptr(projection_matrix));
    glUniformMatrix4fv(glGetUniformLocation(_program, "modelview_matrix"), 1, GL_FALSE, glm::value_ptr(_modelViewMatrix));
    //glUniform1i(glGetUniformLocation(_program, "planetTex"), 0);

    VERIFY(CG::checkError());

    glUniform3fv(glGetUniformLocation(_program, "lightPosition"), 1, glm::value_ptr(lightPos));

    glm::vec3 La(1.0f);
    glUniform3fv(glGetUniformLocation(_program, "La"), 1, glm::value_ptr(La));
    glm::vec3 Ls(1.0f);
    glUniform3fv(glGetUniformLocation(_program, "Ls"), 1, glm::value_ptr(Ls));
    glm::vec3 Ld(1.0f);
    glUniform3fv(glGetUniformLocation(_program, "Ld"), 1, glm::value_ptr(Ld));
    float shininess = 1.f;
    glUniform1f(glGetUniformLocation(_program, "shininess"), shininess);
    glm::vec3 kd(1.0f);
    glUniform3fv(glGetUniformLocation(_program, "kd"), 1, glm::value_ptr(kd));
    glm::vec3 ks(1.0f);
    glUniform3fv(glGetUniformLocation(_program, "ks"), 1, glm::value_ptr(ks));
    glm::vec3 ka(1.0f);
    glUniform3fv(glGetUniformLocation(_program, "ka"), 1, glm::value_ptr(ka));

    // call draw
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    //glDrawArrays(GL_TRIANGLE_STRIP,0,positions.size());
    //glDrawArrays(GL_TRIANGLE_STRIP, 0, positions.size ());

    // unbin vertex array object
    glBindVertexArray(0);

    // check for errors
    VERIFY(CG::checkError());
}

void Planet::update(float elapsedTimeMs, glm::mat4 modelViewMatrix)
{

    float time = elapsedTimeMs/1000.;
    ///TODO: calculate global rotation_modelViewMatrix

    ///TODO: update all drawables that belong to the  child planet

    // calculate new local rotation
    _localRotation += time * _spin;

    // keep rotation between 0 and 360
    while(_localRotation >= 2*M_PI_2)
        _localRotation -= 2*M_PI_2;
    while(_localRotation < 0.0f)
        _localRotation += 2*M_PI_2;

    _globalRotation += time *_orbfreq;

    while(_globalRotation >= 2*M_PI_2)
        _globalRotation -= 2*M_PI_2;
    while(_globalRotation < 0.0f)
        _globalRotation += 2*M_PI_2;

    //transformations
    std::stack<glm::mat4> modelview_stack;

    modelview_stack.push(modelViewMatrix);

    //global rotation
    modelview_stack.top() = glm::rotate(modelview_stack.top(), _globalRotation, glm::vec3(0,1,0));


    //translate from origin to position according to accumulated distances from parents
    float distFromOrigin  = _distance;
    glm::vec3 distVector  = glm::vec3(sin(_orbphase), 0, cos(_orbphase))*glm::vec3(distFromOrigin,0,distFromOrigin);
    modelview_stack.top() = glm::translate(modelview_stack.top(), distVector);

    // rotate around own y-axis i.e. local rotation
    modelview_stack.top() = glm::rotate(modelview_stack.top(), glm::radians(_localRotation), glm::vec3(0,1,0));

    _modelViewMatrix = glm::mat4(modelview_stack.top());

    modelview_stack.pop();
}

GLuint Planet::loadTexture()
{
    glGenTextures(1,&textureID);
    glBindTexture(GL_TEXTURE_2D,textureID);

    Image image(_textureLocation);

    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,image.getWidth(),image.getHeight(),0,GL_RGBA,GL_UNSIGNED_BYTE,image.getData());

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenerateMipmap(GL_TEXTURE_2D);

    VERIFY(CG::checkError());
}

std::string Planet::getVertexShader() const
{
    return Drawable::loadShaderFile(":/shader/planet.vs.glsl");

}

std::string Planet::getFragmentShader() const
{
    return Drawable::loadShaderFile(":/shader/planet.fs.glsl");
}

void Planet::createObject(){

    // clear memory of prev arrays
    positions.clear();
    indices.clear();
    texcoords.clear();
    normals.clear();

    int subdivs= 4;

    int triangles = 2* pow(4,subdivs);
    float r=_radius;

    const int Nphis = pow(2,subdivs+1);
    const int Nthetas = pow(2,subdivs);

    unsigned int indexCounter=0;

    for(int itheta = 0; itheta < Nthetas; itheta++)
    {
        for(int iphi = 0; iphi < Nphis; iphi++)
        {
//          float theta = a*v+a;
            float theta1 = M_PI - (Nthetas - itheta -1) * (M_PI/Nthetas) ;
            float theta2 = M_PI - (Nthetas - itheta -1) * (M_PI/Nthetas) ;
            float theta3 = M_PI - (Nthetas - itheta) * (M_PI/Nthetas) ;
            float theta4 = M_PI - (Nthetas - itheta) * (M_PI/Nthetas) ;

            float phi1;
            float phi2;
            float phi3;
            float phi4;

            if(itheta%2==0)
            {
                phi1 = iphi * (2* M_PI)/Nphis ; //lower left point
                phi2 = (iphi+1.) * (2* M_PI)/Nphis ; //
                phi3 = (iphi+1./2.) * (2* M_PI)/Nphis ; //
                phi4 = (iphi+3./2.) * (2* M_PI)/Nphis ; //upper right point
            }
            else
            {
                phi1 = (iphi-1./2.) * (2* M_PI)/Nphis ; //lower left point
                phi2 = (iphi+1./2.) * (2* M_PI)/Nphis ; //
                phi3 = (iphi) * (2* M_PI)/Nphis ; //
                phi4 = (iphi+1.) * (2* M_PI)/Nphis ; //upper right point
            }

            float x1 = r * sin(theta1)*cos(phi1);
            float y1 = r * sin(theta1)*sin(phi1);
            float z1 = r * cos(theta1);

            float x2 = r * sin(theta2)*cos(phi2);
            float y2 = r * sin(theta2)*sin(phi2);
            float z2 = r * cos(theta2);

            float x3 = r * sin(theta3)*cos(phi3);
            float y3 = r * sin(theta3)*sin(phi3);
            float z3 = r * cos(theta3);

            float x4 = r * sin(theta4)*cos(phi4);
            float y4 = r * sin(theta4)*sin(phi4);
            float z4 = r * cos(theta4);

            glm::mat3 rotx = glm::mat3(glm::vec3(1,0,0),glm::vec3(0,0,1),glm::vec3(0,-1,0));

            positions.push_back(glm::vec3(x1,y1,z1)*rotx);
            positions.push_back(glm::vec3(x2,y2,z2)*rotx);
            positions.push_back(glm::vec3(x3,y3,z3)*rotx);
            positions.push_back(glm::vec3(x4,y4,z4)*rotx);

            texcoords.push_back(glm::vec2(phi1/(2*M_PI),theta1/M_PI));
            texcoords.push_back(glm::vec2(phi2/(2*M_PI),theta2/M_PI));
            texcoords.push_back(glm::vec2(phi3/(2*M_PI),theta3/M_PI));
            texcoords.push_back(glm::vec2(phi4/(2*M_PI),theta4/M_PI));

            normals.push_back(glm::normalize(glm::vec3(x1,y1,z1)*rotx));
            normals.push_back(glm::normalize(glm::vec3(x2,y2,z2)*rotx));
            normals.push_back(glm::normalize(glm::vec3(x3,y3,z3)*rotx));
            normals.push_back(glm::normalize(glm::vec3(x4,y4,z4)*rotx));

            indices.push_back(indexCounter);
            indices.push_back(indexCounter+1);
            indices.push_back(indexCounter+2);

            indices.push_back(indexCounter+2);
            indices.push_back(indexCounter+1);
            indices.push_back(indexCounter+3);

            indexCounter+=4;
        }
     }

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

    // fill texture buffer with data
    GLuint texture_buffer;
    glGenBuffers(1, &texture_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, texture_buffer);
    glBufferData(GL_ARRAY_BUFFER, texcoords.size() * 3 * sizeof(float), texcoords.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_TRUE, 0, 0);
    glEnableVertexAttribArray(1);

    GLuint normal_buffer;
    glGenBuffers(1, &normal_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * 3 * sizeof(float), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_TRUE, 0, 0);
    glEnableVertexAttribArray(2);

    GLuint index_buffer;
    glGenBuffers(1, &index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // unbind vertex array object
    glBindVertexArray(0);
    // delete buffers (the data is stored in the vertex array object)
    glDeleteBuffers(1, &position_buffer);
    glDeleteBuffers(1, &index_buffer);
    glDeleteBuffers(1, &texture_buffer);


    // check for errors
    VERIFY(CG::checkError());

}

Planet::~Planet(){
}



