#include "mainview.h"
#include "vertex.h"

/**
 * @brief MainView::MainView Mainview constructor. The ": QOpenGLWidget(parent)"
 * notation means calling the super class constructor of QOpenGLWidget with the
 * parameter "parent".
 * @param parent Parent widget.
 */
MainView::MainView(QWidget *parent) : QOpenGLWidget(parent) {
  qDebug() << "MainView constructor";
}

/**
 * @brief MainView::~MainView Destructor. Can be used to free memory.
 */
MainView::~MainView() {
  qDebug() << "MainView destructor";
  glDeleteBuffers(1, &vboName);
  glDeleteVertexArrays(1, &vaoName);

  makeCurrent();
}

/**
 * @brief MainView::initializeGL Initialize the necessary OpenGL context.
 */
void MainView::initializeGL() {
  qDebug() << ":: Initializing OpenGL";
  initializeOpenGLFunctions();

  connect(&debugLogger, SIGNAL(messageLogged(QOpenGLDebugMessage)), this,
          SLOT(onMessageLogged(QOpenGLDebugMessage)), Qt::DirectConnection);

  if (debugLogger.initialize()) {
    qDebug() << ":: Logging initialized";
    debugLogger.startLogging(QOpenGLDebugLogger::SynchronousLogging);
  }

  QString glVersion{reinterpret_cast<const char *>(glGetString(GL_VERSION))};
  qDebug() << ":: Using OpenGL" << qPrintable(glVersion);

  vertex a = {.x=-0.5, .y=-0.5, .r=1, .g=0, .b=0};
  vertex b = {.x=0.5, .y=-0.5, .r=0, .g=1, .b=0};
  vertex c = {.x=0, .y=0.5, .r=0, .g=0, .b=1};
  vertex vec[3] = {a, b, c};

  glGenBuffers(1,&vboName);
  glGenVertexArrays(1,&vaoName);

  //shaderProgram = new QOpenGLShaderProgram(this);
  shaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vertshader.glsl");
  shaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragshader.glsl");

  glBindVertexArray(vaoName);
  glBindBuffer(GL_ARRAY_BUFFER,vboName);
  glBufferData(GL_ARRAY_BUFFER,sizeof(vec),vec,GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,sizeof(vertex),(void *)0);
  glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(vertex),(void *)offsetof(vertex, r));
}

/**
 * @brief MainView::resizeGL Called when the window is resized.
 * @param newWidth The new width of the window.
 * @param newHeight The new height of the window.
 */
void MainView::resizeGL(int newWidth, int newHeight) {
  Q_UNUSED(newWidth)
  Q_UNUSED(newHeight)
}

/**
 * @brief MainView::paintGL Draw function. TODO.
 */
void MainView::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(vaoName);
    glClearColor(0.5,0.5,0.5,0.6);
    shaderProgram.bind();
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

/**
 * @brief MainView::onMessageLogged Debug utility method.
 * @param Message The message to be logged.
 */
void MainView::onMessageLogged(QOpenGLDebugMessage Message) {
  qDebug() << " → Log:" << Message;
}
