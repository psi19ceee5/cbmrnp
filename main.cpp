/*
 * Copyright (C) 2013, 2014
 * Computer Graphics Group, University of Siegen
 * Written by Martin Lambers <martin.lambers@uni-siegen.de>
 * All rights reserved.
 */


#include <GL/glew.h>

#include <QtPlugin>
#if defined(QT_STATICPLUGIN) && defined(_WIN32)
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
#endif
#include <QApplication>
#include <QGLFormat>

#include <GLWidget.hpp>

#include <iostream>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    GLWidget::setGLFormat(QGL::DoubleBuffer | QGL::Rgba);

    GLWidget glwidget;
    glwidget.resize(1080, 720);
    glwidget.move(0,1080);
    glwidget.show();

    QTimer idleTimer;
    QObject::connect(&idleTimer, SIGNAL(timeout()), &glwidget, SLOT(animateGL()));
    idleTimer.start();

    return app.exec();
}
