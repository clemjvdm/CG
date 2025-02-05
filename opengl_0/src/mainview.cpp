#include "mainview.h"

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
void MainView::paintGL() {}

/**
 * @brief MainView::onMessageLogged Debug utility method.
 * @param Message The message to be logged.
 */
void MainView::onMessageLogged(QOpenGLDebugMessage Message) {
  qDebug() << " → Log:" << Message;
}
