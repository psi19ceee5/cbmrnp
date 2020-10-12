#include <QApplication>

#include "gui/glwidget.hpp"
#include "gui/cli.h"


int main(int argc, char *argv[])
{
    cli theCli(argc, argv);
    if(!theCli.continueRun())
	return 1;

    QApplication app(argc, argv);

    GLWidget::setGLFormat();

    GLWidget glwidget;
    glwidget.resize(1080, 720);
    glwidget.move(0,1080);
    glwidget.show();

    return app.exec();
}
