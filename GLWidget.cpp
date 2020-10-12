#include <GLWidget.hpp>

#include <glbase/gltool.hpp>

#include <vs.glsl.h>
#include <fs.glsl.h>

#include <iostream>

#define _USE_MATH_DEFINES
#include <math.h>

GLWidget::GLWidget() : QGLWidget(static_cast<QWidget*>(0)), prg(0), vao(0)
{
    t_start = std::chrono::high_resolution_clock::now();
    if (!context()->isValid() || context()->format().majorVersion() != 4)
    {
        QMessageBox::critical(this, "Error", "Cannot get a valid OpenGL 4 context.");
        exit(1);
    }
}

void GLWidget::initializeGL()
{
    // Initialize OpenGL extensions
    glewExperimental = GL_TRUE; // otherwise some function pointers are NULL...
    glewInit();
    glGetError(); // clear an error flag that glewInit() might leave behind
    VERIFY(CG::checkError());

    // Set up the programmable pipeline
    if (prg == 0) {
        GLuint vs = CG::createCompileShader(GL_VERTEX_SHADER, VS_GLSL_STR); VERIFY(vs);
        GLuint fs = CG::createCompileShader(GL_FRAGMENT_SHADER, FS_GLSL_STR); VERIFY(fs);
        prg = glCreateProgram();
        glAttachShader(prg, vs);
        glAttachShader(prg, fs);
        glBindFragDataLocation(prg, 0, "fcolor");
        prg = CG::linkProgram(prg); VERIFY(prg);
    }
}

void GLWidget::initializeBuffers(glm::vec3 vertexArray[], GLuint vertexSize,
                                 glm::vec3 colorArray[],  GLuint colorSize)
{
    if (vao == 0) // if vertex array object not initialized
    {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        GLuint posAttrib = glGetAttribLocation(prg, "pos");
        GLuint colorAttrib = glGetAttribLocation(prg, "color");

        // initialize vertex buffer
        GLuint vertexBuffer;
        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, vertexSize, (GLfloat*) vertexArray, GL_STATIC_DRAW);
        glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(posAttrib);

        // initialize color buffer
        GLuint colorBuffer;
        glGenBuffers(1, &colorBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
        glBufferData(GL_ARRAY_BUFFER, colorSize, (GLfloat*) colorArray, GL_STATIC_DRAW);
        glVertexAttribPointer(colorAttrib, 3, GL_FLOAT, GL_TRUE, 0, 0);
        glEnableVertexAttribArray(colorAttrib);


    }
}

void GLWidget::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);
}

void GLWidget::paintGL()
{
    // Geometry to render: a triangle
    static glm::vec3 vertices[] = {
        glm::vec3( 0.,  std::sqrt(3)/2-sin(M_PI*(15./180.)),  0.0), glm::vec3( std::cos(M_PI*(15./180.)),  -std::sin(M_PI*(15./180.)),  0.0), glm::vec3(-std::cos(M_PI*(15./180.)),  -std::sin(M_PI*(15./180.)),  0.0)

    };
    // vertex colors
    static glm::vec3 colors[] = {
        glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)
    };

    // Set up a vertex array object and assign the vertex arrays for the geometry
    initializeBuffers(vertices, sizeof(vertices), colors, sizeof(colors));

    // Render: clear framebuffer ressources
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // Render: draw geometry
    glUseProgram(prg);
    glBindVertexArray(vao);

    // attach uniform time
    GLint unitime = glGetUniformLocation(prg, "unitime");
    glUniform1f(unitime, time);

    glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices)/sizeof(glm::vec3));

    VERIFY(CG::checkError());
}

void GLWidget::animateGL()
{
    auto tnow = std::chrono::high_resolution_clock::now();
    time = std::chrono::duration_cast<std::chrono::duration<float>>(tnow - t_start).count();
    updateGL();
}

/* Qt mouse and keyboard events */
void GLWidget::keyPressEvent(QKeyEvent* event)
{
    static bool fullscreen = false;
    switch (event->key()) {
    case Qt::Key_F:
        if (fullscreen) {
            showNormal();
            fullscreen = false;
        } else {
            showFullScreen();
            fullscreen = true;
        }
        break;
    case Qt::Key_Escape:
    case Qt::Key_Q:
        close();
        break;
    }
}
