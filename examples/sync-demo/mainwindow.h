#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QVector>

#include "QCustomPlot.h"
#include "QCPSyncer.h"

class MainWindow : public QMainWindow
{
  Q_OBJECT
public:
  explicit MainWindow(QWidget *parent = nullptr);

private slots:
  void updateData();

private:
  QVector<QCustomPlot*> mPlots;
  QVector<QCPGraph*> mSinGraphs;
  QVector<QCPGraph*> mCosGraphs;
  QCPSyncer *mSyncer;
  QTimer mTimer;
  double mTime;
};

#endif // MAINWINDOW_H
