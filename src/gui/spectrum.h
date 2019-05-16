#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include "falcon/definitions.h"
#include "QMouseEvent"
#include "QWheelEvent"
#include <qopengl.h>

class Spectrum : public QOpenGLWidget {
  Q_OBJECT
public:
  explicit Spectrum(QWidget *parent = 0);
  virtual ~Spectrum();
  void addLine(const float* data);
  void scroll_up();
  void scroll_down();

  float intensity_factor = 1.0; // MoHAcks
  float min_intensity = 0.0;
  float max_intensity = 500000.0;
  bool paused = false;
  int pos_y_buff = 0;  // Position Buffer for scrolling
  int view_port = SPECTROGRAM_LINE_COUNT - SPECTROGRAM_LINE_SHOWN - 1; // Position for scrolling

protected:
  void initializeGL();
  void resizeGL(int width, int height);
  void paintGL();
  void mousePressEvent(QMouseEvent *event) override;    // Klick and scroll per mousewheel
  //void wheelEvent(QWheelEvent *event) override;         //

private:
  GLuint textureHandles[2] = {0,0};
  unsigned int hasTextures;
  GLubyte *textureBuffer;



  int nextLine = SPECTROGRAM_LINE_COUNT - 1; //0

  void initializeTextureBuffer();
  void loadTexture();
  void drawSpectrogram();
  void intensityToRGB(float intensity, GLubyte *rgbaOut);  

signals:
  void subwindow_click();
  //void subwindow_scroll(QMouseEvent *event);

};


#endif // GLWIDGET_H
