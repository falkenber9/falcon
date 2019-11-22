/*
 * Copyright (c) 2019 Robert Falkenberg.
 *
 * This file is part of FALCON 
 * (see https://github.com/falkenber9/falcon).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 */
#pragma once

#include <QOpenGLWidget>
#include "falcon/definitions.h"
#include "QMouseEvent"
#include "QWheelEvent"
#include "QElapsedTimer"
#include <qopengl.h>
#include "settings.h"

#define SPECTROGRAM_NOF_TEXTURES 2

#define PRINT_FPS 0

#if PRINT_FPS
#include "QTimer"
#endif

class Spectrum : public QOpenGLWidget {
  Q_OBJECT
public:
  explicit Spectrum(QWidget *parent = nullptr, Settings *glob_settings = nullptr);
  virtual ~Spectrum() override;
  void addLine(const uint16_t* data);
  void scroll_up();
  void scroll_down();

  float intensity_factor = 1.0;
  float min_intensity = 0.0;
  float max_intensity = 500000.0;
  bool paused = false;
  int pos_y_buff = 0;  // Position buffer for scrolling
  int view_port = SPECTROGRAM_LINE_COUNT - SPECTROGRAM_LINE_SHOWN - 1; // Position for scrolling    

protected:
  void initializeGL() override;
  void resizeGL(int width, int height) override;
  void paintGL() override;
  void mousePressEvent(QMouseEvent *event) override;

private:
  Settings *settings;
  GLuint textureHandles[SPECTROGRAM_NOF_TEXTURES];
  GLubyte *textureBuffer;  
  int nextLine = SPECTROGRAM_LINE_COUNT - 1;
  volatile bool textureUpdateNeeded;
  QElapsedTimer lastUpdate;
#if PRINT_FPS
  QTimer fps_timer;
  uint32_t dataChangeCount;
  uint32_t textureUpdateCount;
  uint32_t textureDrawCount;
#endif

  void initializeTextureBuffer();
  void loadTexture();
  void drawSpectrogram();
  void intensityToRGB(float intensity, GLubyte *rgbaOut);  

signals:
  void subwindow_click();

public slots:
#if PRINT_FPS
  void printFrameCount();
#endif

};
