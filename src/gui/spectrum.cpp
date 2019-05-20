#include "spectrum.h"
#include "settings.h"

#include <math.h>
#include <climits>
#include "QDebug"
#include "QWidget"
#include "QMouseEvent"
#define speed 4   // EVtl into options (Mousewheel)


Spectrum::Spectrum(QWidget *parent, Settings *glob_settings) :
  QOpenGLWidget(parent),
  hasTextures(0),
  textureBuffer(new GLubyte[SPECTROGRAM_LINE_COUNT * glob_settings->glob_args.spectrum_args.spectrum_line_width * 4])

{
    settings = glob_settings;
}

Spectrum::~Spectrum() {
  glDeleteTextures(hasTextures, textureHandles);
  delete [] textureBuffer;
}

void Spectrum::initializeGL() {
  initializeTextureBuffer();
  loadTexture();
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
}

void Spectrum::resizeGL(int width, int height) {
  glViewport(0, 0, (GLint)width, (GLint)height);
}

void Spectrum::initializeTextureBuffer() {
  // Init Texture with 0

  for(int i = 0; i < SPECTROGRAM_LINE_COUNT * settings->glob_args.spectrum_args.spectrum_line_width * 4; i++){
    textureBuffer[i] = 0;
  }
  loadTexture();
}

void Spectrum::intensityToRGB(float intensity, GLubyte *rgbaOut) {
  //Mo HAcks:

 /* double map_pos = (intensity / (float)USHRT_MAX) * (float)UCHAR_MAX;
  double n = (4.0 * map_pos) / UCHAR_MAX;*/

  if(intensity > max_intensity) intensity = max_intensity;
  double n = intensity_factor * 4.0 *((intensity - min_intensity) / (float)USHRT_MAX);   //Added dynamic range: intensity factor = 4.5 * USHRT_MAX / (MAX - MIN) [Factor is calculated outside of Spectrum] // * 4.0

  rgbaOut[0] = ((float)UCHAR_MAX)*std::min(std::max(std::min(n-1.5,-n+4.5),0.0),1.0);
  rgbaOut[1] = ((float)UCHAR_MAX)*std::min(std::max(std::min(n-0.5,-n+3.5),0.0),1.0);
  rgbaOut[2] = ((float)UCHAR_MAX)*std::min(std::max(std::min(n+0.5,-n+2.5),0.0),1.0);
  rgbaOut[3] = 0xFF;                                                                  //Transparenz immer 0.

  //if(intensity != 0)qDebug() << "Float Value:"<< intensity << " R:" << rgbaOut[0] << " G:" << rgbaOut[1] << "B:"<< rgbaOut[2] << " N:" << n;
}

void Spectrum::addLine(const float *data) {

  if(!paused){
    GLubyte *buffer = textureBuffer + nextLine * settings->glob_args.spectrum_args.spectrum_line_width * 4;
    for (unsigned int col = 0; col < settings->glob_args.spectrum_args.spectrum_line_width; ++col)
    {
      intensityToRGB(data[col], buffer);
      buffer += 4;
    }
    nextLine--;
    if(nextLine < 0) nextLine = SPECTROGRAM_LINE_COUNT - 1;
  }
}

void Spectrum::loadTexture() {
  glDeleteTextures(hasTextures, textureHandles);
  hasTextures = 0;
  glGenTextures(1, textureHandles);
  hasTextures = 1;


  if(SPECTROGRAM_LINE_SHOWN + view_port <= nextLine)
  {
    // Newer Buffer
    glBindTexture(GL_TEXTURE_2D, textureHandles[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D,
                 0,  GL_RGBA,
                 settings->glob_args.spectrum_args.spectrum_line_width, SPECTROGRAM_LINE_SHOWN,
                 0, GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 textureBuffer + (nextLine - (view_port + SPECTROGRAM_LINE_SHOWN)) * settings->glob_args.spectrum_args.spectrum_line_width * 4);
  }
  else if(view_port >= nextLine){

    // Newer Buffer
    glBindTexture(GL_TEXTURE_2D, textureHandles[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D,
                 0,  GL_RGBA,
                 settings->glob_args.spectrum_args.spectrum_line_width, SPECTROGRAM_LINE_SHOWN,
                 0, GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 textureBuffer + (SPECTROGRAM_LINE_COUNT - (view_port - nextLine) - SPECTROGRAM_LINE_SHOWN) * settings->glob_args.spectrum_args.spectrum_line_width * 4);
  }
  else if(SPECTROGRAM_LINE_SHOWN + view_port > nextLine && view_port < nextLine){  // evtl withour && view_port < nextLine
    // Newer Buffer
    glBindTexture(GL_TEXTURE_2D, textureHandles[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D,
                 0,  GL_RGBA,
                 settings->glob_args.spectrum_args.spectrum_line_width, nextLine - view_port,
                 0, GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 textureBuffer);
    // Older Buffer
    glBindTexture(GL_TEXTURE_2D, textureHandles[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D,
                 0,  GL_RGBA,
                 settings->glob_args.spectrum_args.spectrum_line_width, SPECTROGRAM_LINE_SHOWN - (nextLine - view_port),
                 0, GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 textureBuffer + (SPECTROGRAM_LINE_COUNT - (SPECTROGRAM_LINE_SHOWN - (nextLine - view_port))) * settings->glob_args.spectrum_args.spectrum_line_width * 4);
  }
}

void Spectrum::drawSpectrogram() {

  if(SPECTROGRAM_LINE_SHOWN + view_port <= nextLine || view_port >= nextLine){

    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    // Draw newer buffer
    float startX = 0;
    float startY = 0;
    float endX = 1.0;
    float endY = 1.0;    
    glBindTexture(GL_TEXTURE_2D, textureHandles[0]);
    glBegin(GL_QUADS);
    glTexCoord2d(0.0, 1.0);
    glVertex3f(startX, startY, 0.0);
    glTexCoord2d(1.0, 1.0);
    glVertex3f(endX, startY, 0.0);
    glTexCoord2d(1.0, 0.0);
    glVertex3f(endX, endY, 0.0);
    glTexCoord2d(0.0, 0.0);
    glVertex3f(startX, endY, 0.0);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glFlush();

  }
  else if(SPECTROGRAM_LINE_SHOWN + view_port > nextLine && view_port < nextLine){

    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    // Draw newer buffer
    float startX = 0;
    float startY = 0;
    float endX = 1.0;
    float endY = (float)(nextLine - view_port) / (float)SPECTROGRAM_LINE_SHOWN;
    glBindTexture(GL_TEXTURE_2D, textureHandles[0]);
    glBegin(GL_QUADS);
    glTexCoord2d(0.0, 1.0); glVertex3f(startX, startY, 0.0);
    glTexCoord2d(1.0, 1.0); glVertex3f(endX, startY, 0.0);
    glTexCoord2d(1.0, 0.0); glVertex3f(endX, endY, 0.0);
    glTexCoord2d(0.0, 0.0); glVertex3f(startX, endY, 0.0);
    glEnd();

    // Draw older buffer
    startY = endY;
    endY = 1.0;
    glBindTexture(GL_TEXTURE_2D, textureHandles[1]);
    glBegin(GL_QUADS);
    glTexCoord2d(0.0, 1.0); glVertex3f(startX, startY, 0.0);
    glTexCoord2d(1.0, 1.0); glVertex3f(endX, startY, 0.0);
    glTexCoord2d(1.0, 0.0); glVertex3f(endX, endY, 0.0);
    glTexCoord2d(0.0, 0.0); glVertex3f(startX, endY, 0.0);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glFlush();
  }
}

void Spectrum::paintGL() {
  loadTexture();
  drawSpectrogram();
}

void Spectrum::mousePressEvent(QMouseEvent * event){

   emit subwindow_click();

  /*paused = !paused;
  pos_y_buff = event->y();
  view_port = SPECTROGRAM_LINE_COUNT - SPECTROGRAM_LINE_SHOWN - 1;*/
}

/*void Spectrum::wheelEvent(QWheelEvent *event){

  if(paused){
    if(event->delta() > 0) scroll_up();
    else scroll_down();
  }
}*/

void Spectrum::scroll_up(){

  if(view_port <= SPECTROGRAM_LINE_COUNT - SPECTROGRAM_LINE_SHOWN - speed) view_port += speed;
  // qDebug() << view_port;
  update();
}

void Spectrum::scroll_down(){


  if(view_port >= speed) view_port -= speed;
  //else view_port = 0;

  //qDebug() << view_port;
  update();
}
