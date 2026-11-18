/* QCustomPlot 2.1.1 split source
   Copyright (C) 2011-2022 Emanuel Eichhammer
   Licensed under the GNU General Public License v3, see GPL.txt.
   Automatically split from the amalgamated qcustomplot.h/.cpp; keep code in sync. */

#ifndef QCPSYNCER_P_H
#define QCPSYNCER_P_H

#include "QCPSyncer.h"
#include "QCPItemStraightLine.h"
#include "QCPItemTracer.h"
#include "QCPItemText.h"

#include <QHash>
#include <QList>
#include <QPointer>

//! One draggable vertical ruler with per-graph readout items, spanning several QCustomPlot widgets.
class QCPRuler
{
public:
  QCPRuler();
  ~QCPRuler();

  void setKey(double key);
  double key() const { return mKey; }
  void setVisible(bool visible);
  bool visible() const { return mVisible; }
  void setPen(const QPen &pen);
  QPen pen() const { return mPen; }

  void setReadoutEnabled(QCPGraph *graph, bool enabled);
  bool readoutEnabled(QCPGraph *graph) const;
  void setReadoutFormatter(std::function<QString(double)> formatter);
  void setXLabelFormatter(std::function<QString(double)> formatter);
  void setLabelPlot(QCustomPlot *plot);
  void setLabelAxisRect(QCPAxisRect *rect);

  void attachPlot(QCustomPlot *plot);
  void detachPlot(QCustomPlot *plot);
  void forgetPlot(QCustomPlot *plot);
  void rescanPlot(QCustomPlot *plot);
  void rescanAll();
  void refreshPlot(QCustomPlot *plot);
  void refresh();

  double interpolate(QCPGraph *graph, bool *ok) const;

private:
  struct PlotItems
  {
    QList<QCPItemStraightLine*> lines;
    QList<QCPAxisRect*> lineRects;
    QHash<QCPGraph*, QCPItemTracer*> tracers;
    QHash<QCPGraph*, QCPItemText*> labels;
  };

  void createPlotItems(QCustomPlot *plot);
  void clearReadoutItems(QCustomPlot *plot);
  void clearPlotItems(QCustomPlot *plot);
  void ensureLabelItem();
  void discardLabelItem();
  void updatePlotVisuals(QCustomPlot *plot);
  QString readoutText(QCPGraph *graph, bool *visible) const;
  QString xLabelText(double key, QCPAxis *axis) const;

  double mKey;
  bool mVisible;
  QPen mPen;
  QHash<QCPGraph*, bool> mDisabledGraphs;
  QHash<QCustomPlot*, PlotItems> mItems;
  QPointer<QCustomPlot> mLabelPlot;
  QPointer<QCPAxisRect> mLabelAxisRect;
  QPointer<QCPItemText> mXLabelItem;
  std::function<QString(double)> mReadoutFormatter;
  std::function<QString(double)> mXLabelFormatter;
};

#endif // QCPSYNCER_P_H
