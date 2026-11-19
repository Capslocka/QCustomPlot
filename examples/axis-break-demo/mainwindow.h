#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "QCPAxisBreak.h"
#include "QCustomPlot.h"

class QCPSyncer;

class MainWindow : public QMainWindow
{
  Q_OBJECT
public:
  explicit MainWindow(QWidget *parent = nullptr);

private:
  void addSeries(double phase, const QColor &color, const QString &name);

  QCustomPlot *mPlot;
  QCPSyncer *mSyncer;
  QCPAxisBreak mAxisBreak;
};

#endif // MAINWINDOW_H
