#include "file_input_output.h"
#include <QWidget>
#include <QFileDialog>
#include <QDebug>

QString FileDialog:: openFile(){

  QString filename =  QFileDialog::getOpenFileName(
        this,
        "Open Document",
        QDir::currentPath(),
        "All files (*.*) ;; Sample Files (*.bin *.csv)");

  if( !filename.isNull() )
  {
    qDebug() << "selected file path : " << filename.toUtf8();
    return filename;
  }
  return "Error with File!";
}

