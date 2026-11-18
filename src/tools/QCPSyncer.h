/* QCustomPlot 2.1.1 split source
   Copyright (C) 2011-2022 Emanuel Eichhammer
   Licensed under the GNU General Public License v3, see GPL.txt.
   Automatically split from the amalgamated qcustomplot.h/.cpp; keep code in sync. */

#ifndef QCPSYNCER_H
#define QCPSYNCER_H

#include "QCPGlobal.h"
#include "QCustomPlot.h"
#include "QCPAxis.h"
#include "QCPAxisRect.h"
#include "QCPGraph.h"

#include <functional>

class QCPRuler;

/*!
  Synchronizes the horizontal (key) axis ranges of several QCustomPlot widgets and provides a
  draggable vertical ruler whose intersection with every graph is read out with a tracer and a
  text label.

  Ruler x-axis value labels are shown in a single plot only, see setLabelPlot.
*/
class QCP_LIB_DECL QCPSyncer : public QObject
{
  Q_OBJECT
public:
  explicit QCPSyncer(QObject *parent = nullptr);
  ~QCPSyncer() override;

  void addPlot(QCustomPlot *plot);
  void removePlot(QCustomPlot *plot);
  QList<QCustomPlot*> plots() const;

  //! An empty list restores the default: the bottom axis of every axis rect of every plot.
  void setSyncedAxes(const QList<QCPAxis*> &axes);
  QList<QCPAxis*> syncedAxes() const;

  void setRulerKey(double key);
  double rulerKey() const;
  void setRulerVisible(bool visible);
  bool rulerVisible() const;
  void setRulerPen(const QPen &pen);
  QPen rulerPen() const;

  void setReadoutEnabled(QCPGraph *graph, bool enabled);
  bool readoutEnabled(QCPGraph *graph) const;
  void rescanGraphs();

  void setLabelPlot(QCustomPlot *plot);
  QCustomPlot *labelPlot() const;
  void setLabelAxisRect(QCPAxisRect *rect);
  QCPAxisRect *labelAxisRect() const;

  //! Formats the interpolated graph value at the ruler key. Empty function restores the default.
  void setReadoutFormatter(std::function<QString(double)> formatter);
  //! Formats the ruler x value. Empty function restores the default (datetime tickers use their format).
  void setXLabelFormatter(std::function<QString(double)> formatter);

signals:
  void rulerKeyChanged(double key);
  void syncedRangeChanged(const QCPRange &range);

protected:
  bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
  void onAxisRangeChanged(const QCPRange &range);

private:
  Q_DISABLE_COPY(QCPSyncer)

  QList<QCPAxis*> effectiveSyncedAxes() const;
  void rebuildConnections();
  void connectAxis(QCPAxis *axis);
  void disconnectAxis(QCPAxis *axis);
  void applyRange(const QCPRange &range, QCPAxis *source);
  void updateDragCursor(QCustomPlot *plot, const QPoint &pos);
  QCPAxis *axisAt(QCustomPlot *plot, const QPoint &pos) const;
  void setRulerKeyInternal(double key);
  void queueReplotAll();

  QList<QCustomPlot*> mPlots;
  QList<QCPAxis*> mExplicitSyncedAxes;
  QList<QCPAxis*> mConnectedAxes;
  bool mSyncing;
  QPointer<QCustomPlot> mLabelPlot;
  QPointer<QCPAxisRect> mLabelAxisRect;
  std::function<QString(double)> mReadoutFormatter;
  std::function<QString(double)> mXLabelFormatter;

  QCPRuler *mRuler;
  bool mKeyInitialized;
  bool mDragging;
  QPointer<QCustomPlot> mDragPlot;
  QPointer<QCPAxis> mDragAxis;
};

#endif // QCPSYNCER_H
