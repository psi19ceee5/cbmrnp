#include <iostream>
#include <cmath>
#include <GL/glew.h>

#include "gui/glwidget.hpp"

#include <QMouseEvent>

#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
#include <glm/gtx/transform.hpp>

#include "gui/config.h"

#include "objects/spacetime.h"
#include "objects/skybox.h"
#include "objects/planet.h"

#ifndef M_PI_2
#define M_PI_2 (3.14159265359f * 0.5f)
#endif

// rotate around own y-axis i.e. local rotation

using namespace glm;

GLWidget::GLWidget() : QOpenGLWidget(static_cast<QWidget*>(0)),//static_cast<QWidget*>(0)),
    _updateTimer(this), _stopWatch()
{
    // update the scene periodically
    QObject::connect(&_updateTimer, SIGNAL(timeout()), this, SLOT(animateGL()));
    _updateTimer.start(18);
    _stopWatch.start();

    cameraBelow=false;

    float omega = 4*M_PI_2/15.;

    _skybox    = std::make_shared<Skybox>("Skybox", ":/res/images/stars.bmp");
    _spacetime = std::make_shared<Spacetime>("Spacetime", ":/res/images/gridlines.png");
                                                  //radius //orbital radius //spin //orbital frequency
    _planet1   = std::make_shared<Planet>("planet1", 0.02, 0.05, 4., omega, 0.,      ":/res/images/neutronstar.bmp");
    _planet2   = std::make_shared<Planet>("planet2", 0.02, 0.05, 4., omega, 2*M_PI_2,":/res/images/neutronstar.bmp");
}

void GLWidget::show()
{
    QOpenGLWidget::show();

    // check for a valid context
    if (!isValid() || !context()->isValid() || context()->format().majorVersion() != 4) {
        QMessageBox::critical(this, "Error", "Cannot get a valid OpenGL 4 context.");
        exit(1);
    }
}

void GLWidget::initializeGL()
{
    /* Initialize OpenGL extensions */
    glewExperimental = GL_TRUE; // otherwise some function pointers are NULL...
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
      /* Problem: glewInit failed, something is seriously wrong. */
      fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    }
    glGetError(); // clear a gl error produced by glewInit

    // make sure the context is current
    makeCurrent();

    /// Init all drawables here
    _skybox->init();
    _spacetime->init();
    _planet1->init();
    _planet2->init();
}

void GLWidget::resizeGL(int width, int height)
{
    // update the viewport
    glViewport(0, 0, width, height);
}

void GLWidget::paintGL()
{
    //change to black background
    glClearColor(0.0f,0.0f,0.0f,0.0f);
	
    // Render: set up view
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    GLint m_viewport[4];
    glGetIntegerv( GL_VIEWPORT, m_viewport );

    // calculate projection matrix from resolution
    glm::mat4 projection_matrix = glm::perspective(glm::radians(50.0f),
                float(m_viewport[2])/m_viewport[3],
                0.1f, 100.0f);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    _skybox->draw(projection_matrix);
    _spacetime->draw(projection_matrix);
    _planet1->draw(projection_matrix);
    _planet2->draw(projection_matrix);
    
    glEnable(GL_BLEND);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
}

//default values for camera position
ivec2 mouse_pos = ivec2(-1.0,-1.0);
double theta = 1.15*M_PI_2;
double phi = 0.5*M_PI_2;
double radius= -1.0;

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    //save mouse position when pressing
    ivec2 pos = ivec2(event->pos().x(), event->pos().y());
    mouse_pos = pos;
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event)
{
     mouse_pos = ivec2(-1.0, -1.0); //reset default
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    ivec2 pos = ivec2(event->pos().x(), event->pos().y());
    ivec2 diff = pos - mouse_pos; //move direction
    if (mouse_pos.x > 0 && mouse_pos.y > 0 && (theta+radians(float(diff.y)))>0 && (theta+radians(float(diff.y)))<M_PI )
    {
        theta += radians(2.0*float(diff.y)/(-radius));
        phi += radians(2.0*float(diff.x)/(-radius));
    }
    //update mouse position
    mouse_pos = ivec2(event->pos().x(), event->pos().y());
}

void GLWidget::wheelEvent(QWheelEvent *event)
{
    //handle zoom
    if((radius + radians((float)event->angleDelta().ry())/8.0f)<0) //won't zoom past origin/lookat point
    {
        radius += radians((float)event->angleDelta().ry())/8.0f;
    }
}

void GLWidget::animateGL()
{
    // make the context current in case there are glFunctions called
    makeCurrent();

    // get the time delta
    float timeElapsedMs = _stopWatch.nsecsElapsed() / 1000000.0f;
    // restart stopwatch for next update
    _stopWatch.restart();

    // calculate current modelViewMatrix for the default camera
    glm::vec3 camera = glm::vec3( -radius * sin(theta) * sin(phi), radius * cos(theta), radius * sin(theta)* cos(phi));
    glm::vec3 focus = glm::vec3(0.0f, -0.2f, 0.0f);

    glm::mat4 modelViewMatrix = glm::lookAt(camera, focus, glm::vec3(0.0, 1.0, 0.0));

    // update drawables
    _skybox->update(timeElapsedMs, modelViewMatrix);
    _planet1->update(timeElapsedMs, modelViewMatrix);
    _planet2->update(timeElapsedMs, modelViewMatrix);
    _spacetime->update(timeElapsedMs, modelViewMatrix);
    _spacetime->recreate();

    // update the widget (do not remove this!)
    update();
}



