#include "mainwindow.h"

#include <QCPSyncer.h>

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

#include <cmath>

namespace
{
const double kBreakStart = 2.0;
const double kBreakEnd = 20.0;
const double kDataEnd = 22.0;

void buildRawSeries(double phase, QVector<double> *keys, QVector<double> *values)
{
  keys->clear();
  values->clear();
  for (double t = 0.0; t <= kBreakStart + 1e-9; t += 0.02)
  {
    keys->append(t);
    values->append(std::sin(3.0 * t + phase));
  }
  // Points inside the hidden interval; mapSeries() drops them and inserts a NaN gap instead.
  for (double t = 3.0; t < kBreakEnd; t += 2.5)
  {
    keys->append(t);
    values->append(0.5 * std::cos(t + phase));
  }
  for (double t = kBreakEnd; t <= kDataEnd + 1e-9; t += 0.02)
  {
    keys->append(t);
    values->append(std::sin(3.0 * t + phase));
  }
}
}

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  mPlot(new QCustomPlot(this)),
  mSyncer(new QCPSyncer(this))
{
  const double realLength = kDataEnd - (kBreakEnd - kBreakStart);
  mAxisBreak.setBreaks({QCPRange(kBreakStart, kBreakEnd)});
  mAxisBreak.setBreakWidth(0.02 * realLength);

  addSeries(0.0, QColor(0x1e, 0x88, 0xe5), "sin");
  addSeries(0.6, QColor(0xe5, 0x39, 0x35), "cos");

  mAxisBreak.attachAxis(mPlot->xAxis);

  mPlot->xAxis->setLabel("time (s)");
  mPlot->xAxis->setRange(mAxisBreak.map(0.0) - 0.1, mAxisBreak.map(kDataEnd) + 0.1);
  mPlot->yAxis->setRange(-1.6, 1.6);
  mPlot->legend->setVisible(true);
  mPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

  mSyncer->addPlot(mPlot);
  mSyncer->setLabelPlot(mPlot);
  mSyncer->setKeyRangeLimit(mAxisBreak.map(0.0), mAxisBreak.map(kDataEnd));
  mSyncer->setXLabelFormatter([this](double mappedKey) {
    double key = 0.0;
    if (!mAxisBreak.unmap(mappedKey, &key))
      return QString(); // ruler inside the hidden band: no x label
    return QStringLiteral("t = %1 s").arg(key, 0, 'f', 2);
  });
  mSyncer->addRuler(1, mAxisBreak.map(1.0));
  mSyncer->addRuler(2, mAxisBreak.map(21.0));

  QLabel *hint = new QLabel(
      "<b>Broken axis:</b> the key interval [2, 20] is hidden; points inside it were dropped "
      "and the line is interrupted by a NaN gap. Drag a ruler into the slash band: its readouts "
      "and x label disappear there.", this);
  hint->setWordWrap(true);
  hint->setContentsMargins(6, 6, 6, 0);

  QWidget *central = new QWidget(this);
  QVBoxLayout *layout = new QVBoxLayout(central);
  layout->setContentsMargins(4, 4, 4, 4);
  layout->addWidget(hint);
  layout->addWidget(mPlot, 1);
  setCentralWidget(central);

  mSyncer->setActiveRuler(1);
}

void MainWindow::addSeries(double phase, const QColor &color, const QString &name)
{
  QVector<double> rawKeys;
  QVector<double> rawValues;
  buildRawSeries(phase, &rawKeys, &rawValues);

  QVector<double> keys;
  QVector<double> values;
  mAxisBreak.mapSeries(rawKeys, rawValues, &keys, &values);

  QCPGraph *graph = mPlot->addGraph();
  graph->setName(name);
  graph->setPen(QPen(color, 2));
  graph->setData(keys, values, true);
}
