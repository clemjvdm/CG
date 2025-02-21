#include "mainview.h"
#include "model.h"
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


void MainView::initializePyramid(GLuint VBO, GLuint VAO, QMatrix4x4 *modelTrans, int *vertexCount) {

  // initialize vertices
  Vertex v1(-1,1,1,1,0,0);
  Vertex v2(1,1,1,0,1,0);
  Vertex v3(1,-1,1,1,1,0);
  Vertex v4(-1,-1,1,0,0,1);
  Vertex v5(0,0,-1,1,0,1);

  // create vertex array
  std::vector<Vertex> vertices = {
    v1,v4,v3,
    v1,v3,v2,
    v5,v1,v2,
    v5,v4,v1,
    v2,v3,v5,
    v4,v5,v3,
  };
  *vertexCount = vertices.size();

  // Create VAO & VBO
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  // Initialize buffer data store
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

  // specify and enable vertex attribute pointers
  glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void *)0);
  glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex), (void *)offsetof(Vertex,red));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  // translate pyramid
  modelTrans->setToIdentity();
  modelTrans->translate(-2,0,-6);

}

void MainView::initializeKnot(GLuint VBO, GLuint VAO, QMatrix4x4 *modelTrans, int *vertexCount) {

  // load model
  Model knot(":/models/knot.obj");

  // initialize and fill vertices vector
  std::vector<Vertex> vertices;
  for (QVector3D i : knot.getMeshCoords()) {
      vertices.push_back(Vertex(i.x(), i.y(), i.z(), abs(i.x()), abs(i.y()), abs(i.z())));
  }
  *vertexCount = vertices.size();

  // Create VAO & VBO
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  // Initialize buffer data store
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

  // specify and enable vertex attribute pointers
  glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void *)0);
  glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex), (void *)offsetof(Vertex,red));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  // translate knot
  modelTrans->setToIdentity();
  modelTrans->translate(2,0,-6);
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
  glDeleteBuffers(vbos.size(), vbos.data());
  glDeleteVertexArrays(vaos.size(), vaos.data());
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

  // Each object has his own vao, vbo, transformation matrix and vertex count. All of these are stored in
  // vectors; vaos, vbos, transformations and vertexCounts.
  //
  // This makes it trivial to add more objects.

  // initialize the vbos vector, and get two vbo names
  vbos.resize(2);
  glGenBuffers(2, vbos.data());

  // initialize the vaos vector and get two vao names
  vaos.resize(2);
  glGenVertexArrays(2, vaos.data());

  // initialize the transformations and vertexCount vectors
  transformations.resize(2);
  vertexCounts.resize(2);

  // initialize the pyramid and the knot object
  initializePyramid(vbos[0], vaos[0], &transformations[0], &vertexCounts[0]);
  initializeKnot(vbos[1], vaos[1], &transformations[1], &vertexCounts[1]);

  // An alternative implementation of the above is: since we assume an object is represented by the same index
  // accross all vectors (vaos, vbos, transformations and vertexCounts) we could simply
  // pass said index and let the functions access the vector fields.

  // initialize the projection transformation matrix
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
  shaderProgram.setUniformValue("projectionTransform", projectionTrans);

  // paint all objects
  for (int i=0; i<vaos.size(); i++) {
      shaderProgram.setUniformValue("modelTransform", transformations[i]);
      glBindVertexArray(vaos[i]);
      glDrawArrays(GL_TRIANGLES, 0, vertexCounts[i]);
  }

  shaderProgram.release();
}

/**
 * @brief MainView::resizeGL Called upon resizing of the screen.
 *
 * @param newWidth The new width of the screen in pixels.
 * @param newHeight The new height of the screen in pixels.
 */
void MainView::resizeGL(int newWidth, int newHeight) {
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

  // go over all transformation matrices and modify rotation
  for (QMatrix4x4 &transformation : transformations) {
    QVector3D translation = transformation.column(3).toVector3D(); // extract translation
    float scale = transformation.column(0).length(); // extract scale

    // clear rotation by removing all transformations and re-applying translation and scaling
    transformation.setToIdentity();
    transformation.translate(translation);
    transformation.scale(scale);

    // rotate model by desired amount
    transformation.rotate(rotateX, 1, 0, 0);
    transformation.rotate(rotateY, 0, 1, 0);
    transformation.rotate(rotateZ, 0, 0, 1);
  }

  update();
}

/**
 * @brief MainView::setScale Changes the scale of the displayed objects.
 * @param scale The new scale factor. A scale factor of 1.0 should scale the
 * mesh to its original size.
 */
void MainView::setScale(float scale) {
  qDebug() << "Scale changed to " << scale;

  // for all transformation matrices update scaling
  for (QMatrix4x4 &transformation : transformations) {
      float currentScale = transformation.column(0).length(); //extract current scale
      transformation.scale(1/currentScale); // reset scale
      transformation.scale(scale); // apply desired scaling
  }
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
