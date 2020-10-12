/*
 * Copyright (C) 2014
 * Computer Graphics Group, University of Siegen
 * Written by Martin Lambers <martin.lambers@uni-siegen.de>
 * All rights reserved.
 */

#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <cstdio>
#include <cstdlib>

#include <GL/glew.h>

#include <QGLWidget>
#include <QMessageBox>
#include <QTimer>
#include <QKeyEvent>

#define GLM_FORCE_RADIANS
#include <glm/gtc/type_ptr.hpp>

#include <chrono>

class GLWidget : public QGLWidget
{
    Q_OBJECT

public:
    static void setGLFormat(QGL::FormatOptions format)
    {
        QGLFormat qglFormat(format);
        qglFormat.setSwapInterval(::getenv("COREGL_FPS") ? 0 : 1);
        qglFormat.setVersion(4, 0);
        qglFormat.setProfile(QGLFormat::CoreProfile);
        QGLFormat::setDefaultFormat(qglFormat);
    }

    GLWidget();

    virtual ~GLWidget() {}

    virtual void initializeGL();
    virtual void resizeGL(int width, int height);
    virtual void paintGL();

public slots:
    void animateGL();

protected:
    virtual void keyPressEvent(QKeyEvent* event);
    //virtual void mousePressEvent(QMouseEvent* event);
    //virtual void mouseMoveEvent(QMouseEvent* event);
    //virtual void wheelEvent(QWheelEvent* event);

    void initializeBuffers(glm::vec3 vertexArray[],  GLuint vertexSize, glm::vec3 colorArray[],   GLuint colorSize);

    // program handle
    GLuint prg;
    GLuint vao;

    std::chrono::time_point<std::chrono::high_resolution_clock> t_start;
    float time;

};

#endif
