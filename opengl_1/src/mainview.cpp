#include "mainview.h"
#include "triangle.h"

#include <QDateTime>

#include <cstddef>
/**
 * @brief MainView::MainView Constructs a new main view.
 *
 * @param parent Parent widget.
 */
MainView::MainView(QWidget *parent) : QOpenGLWidget(parent) {
  qDebug() << "MainView constructor";

  connect(&timer, SIGNAL(timeout()), this, SLOT(update()));
}

/**
 * @brief MainView::~MainView
 *
 * Destructor of MainView
 * This is the last function called, before exit of the program
 * Use this to clean up your variables, buffers etc.
 *
 */
MainView::~MainView() {
  qDebug() << "MainView destructor";
  glDeleteBuffers(1, &VBO);
  glDeleteVertexArrays(1, &VAO);
  makeCurrent();
}

// --- OpenGL initialization

/**
 * @brief MainView::initializeGL Called upon OpenGL initialization
 * Attaches a debugger and calls other init functions.
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

  // Enable depth buffer
  glEnable(GL_DEPTH_TEST);

  // Enable backface culling
  glEnable(GL_CULL_FACE);

  // Default is GL_LESS
  glDepthFunc(GL_LEQUAL);

  // Set the color to be used by glClear. This is, effectively, the background
  // color.
  glClearColor(0.37f, 0.42f, 0.45f, 0.0f);


  //Create the triangle vertices with the appropriate colors
  Vertex v1(-1,1,1,1,0,0);
  Vertex v2(1,1,1,0,1,0);
  Vertex v3(1,-1,1,1,1,0);
  Vertex v4(-1,-1,1,0,0,1);
  Vertex v5(0,0,-1,1,0,1);

  //Create the Vertex array
  std::vector<Vertex> vertices = {
    //In anti-clockwise specification
    //Bottom two vertices
    v1,v4,v3,
    v1,v3,v2,
    //Depth vertices
    v5,v1,v2,
    v5,v4,v1,
    v2,v3,v5,
    v4,v5,v3,
  };

  //Create the VAO and VBO for the pyramid
  glGenBuffers(1, &VBO);
  glGenVertexArrays(1, &VAO);


  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);


  glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void *)0);
  glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex), (void *)offsetof(Vertex,red));

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  modelTrans.setToIdentity();
  modelTrans(0,3) = -2.0f;
  modelTrans(1,3) = 0.0f;
  modelTrans(2,3) = -6.0f;

  projectionTrans.setToIdentity();
  projectionTrans.perspective(60, 1, 0.2,20);
  createShaderProgram();
}

/**
 * @brief MainView::createShaderProgram Creates a new shader program with a
 * vertex and fragment shader.
 */
void MainView::createShaderProgram() {
  shaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex,
                                        ":/shaders/vertshader.glsl");
  shaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment,
                                        ":/shaders/fragshader.glsl");
  shaderProgram.link();
}

/**
 * @brief MainView::paintGL Actual function used for drawing to the screen.
 *
 */
void MainView::paintGL() {
  // Clear the screen before rendering
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  shaderProgram.bind();
  glDrawArrays(GL_TRIANGLES,0,18);
  shaderProgram.setUniformValue("modelTransform", modelTrans);
  shaderProgram.setUniformValue("projectionTransform", projectionTrans);
  shaderProgram.release();
}

/**
 * @brief MainView::resizeGL Called upon resizing of the screen.
 *
 * @param newWidth The new width of the screen in pixels.
 * @param newHeight The new height of the screen in pixels.
 */
void MainView::resizeGL(int newWidth, int newHeight) {
  // TODO: Update projection to fit the new aspect ratio
  Q_UNUSED(newWidth)
  Q_UNUSED(newHeight)
  projectionTrans.setToIdentity();
  projectionTrans.perspective(60, (float)newWidth/(float)newHeight, 0.2, 20);

}

/**
 * @brief MainView::setRotation Changes the rotation of the displayed objects.
 * @param rotateX Number of degrees to rotate around the x axis.
 * @param rotateY Number of degrees to rotate around the y axis.
 * @param rotateZ Number of degrees to rotate around the z axis.
 */
void MainView::setRotation(int rotateX, int rotateY, int rotateZ) {
  qDebug() << "Rotation changed to (" << rotateX << "," << rotateY << ","
           << rotateZ << ")";

  // extract translation and null last column
  QVector3D translation = modelTrans.column(3).toVector3D();

  // extract scale
  float scale = modelTrans.column(0).length(); // assuming the scale is always proportional along all axes

  // clear rotation by removing all transformations and re-applying translation and scaling
  modelTrans.setToIdentity();
  modelTrans.translate(translation);
  modelTrans.scale(scale);

  // rotate model by desired amount
  modelTrans.rotate(rotateX, 1, 0, 0);
  modelTrans.rotate(rotateY, 0, 1, 0);
  modelTrans.rotate(rotateZ, 0, 0, 1);
  update();
}

/**
 * @brief MainView::setScale Changes the scale of the displayed objects.
 * @param scale The new scale factor. A scale factor of 1.0 should scale the
 * mesh to its original size.
 */
void MainView::setScale(float scale) {
  qDebug() << "Scale changed to " << scale;

  // exract current scale
  float currentScale = modelTrans.column(0).length(); // assuming the scale is always proportional along all axes

  // reset scale
  modelTrans.scale(1/currentScale);

  // scale to desired value
  modelTrans.scale(scale);
  update();
}

/**
 * @brief MainView::onMessageLogged OpenGL logging function, do not change.
 *
 * @param Message The message to be logged.
 */
void MainView::onMessageLogged(QOpenGLDebugMessage Message) {
  qDebug() << " → Log:" << Message;
}
