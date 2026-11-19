#include "mainwindow.h"

#include <QPointer>
#include <QVBoxLayout>
#include <QWidget>

#include <cmath>

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  mSyncer(new QCPSyncer(this)),
  mTime(0)
{
  QWidget *central = new QWidget(this);
  QVBoxLayout *layout = new QVBoxLayout(central);
  layout->setContentsMargins(4, 4, 4, 4);

  const QColor sinColors[3] = {QColor(40, 110, 220), QColor(220, 90, 40), QColor(40, 160, 90)};
  const QColor background[3] = {Qt::white, QColor(245, 248, 255), QColor(250, 250, 240)};

  for (int i = 0; i < 3; ++i)
  {
    QCustomPlot *plot = new QCustomPlot(central);
    plot->setMinimumHeight(180);
    plot->setBackground(QBrush(background[i]));
    plot->xAxis->setLabel("time (s)");
    plot->yAxis->setLabel(QString("plot %1").arg(i + 1));
    plot->xAxis->setRange(0, 10);
    plot->yAxis->setRange(-1.6, 1.6);
    plot->legend->setVisible(true);

    QCPGraph *sinGraph = plot->addGraph();
    sinGraph->setName("sin");
    sinGraph->setPen(QPen(sinColors[i], 2));
    QCPGraph *cosGraph = plot->addGraph();
    cosGraph->setName("cos");
    cosGraph->setPen(QPen(Qt::darkGray, 1, Qt::DashLine));

    mPlots.append(plot);
    mSinGraphs.append(sinGraph);
    mCosGraphs.append(cosGraph);
    mSyncer->addPlot(plot);
    layout->addWidget(plot);
  }

  setCentralWidget(central);
  mSyncer->setLabelPlot(mPlots.first());
  mSyncer->addRuler(1, 5.0);
  mSyncer->setActiveRuler(1);

  connect(&mTimer, &QTimer::timeout, this, &MainWindow::updateData);
  mTimer.start(50);
}

void MainWindow::updateData()
{
  mTime += 0.05;
  for (int i = 0; i < mPlots.size(); ++i)
  {
    mSinGraphs[i]->addData(mTime, std::sin(mTime * 1.5 + i));
    mCosGraphs[i]->addData(mTime, 0.7 * std::cos(mTime + i * 0.5));
    mSinGraphs[i]->data()->removeBefore(mTime - 20);
    mCosGraphs[i]->data()->removeBefore(mTime - 20);
  }

  if (mTime > 10)
    mPlots.first()->xAxis->setRange(mTime - 10, mTime);

  for (QCustomPlot *plot : mPlots)
    plot->replot(QCustomPlot::rpQueuedReplot);
}
