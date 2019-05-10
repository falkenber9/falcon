#ifndef INPUTOUTPUT_H
#define INPUTOUTPUT_H

#include <QWidget>
#include <QFileDialog>
#include <QDebug>

class FileDialog : public QWidget
{
public:
  QString openFile();
};

#endif // INPUTOUTPUT_H
