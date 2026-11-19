/* QCustomPlot 2.1.1 split source
   Copyright (C) 2011-2022 Emanuel Eichhammer
   Licensed under the GNU General Public License v3, see GPL.txt.
   Automatically split from the amalgamated qcustomplot.h/.cpp; keep code in sync. */

#ifndef QCPSYNCER_H
#define QCPSYNCER_H

#include "QCPGlobal.h"
#include "QCPRuler.h"
#include "QCustomPlot.h"
#include "QCPAxis.h"
#include "QCPAxisRect.h"
#include "QCPGraph.h"

#include <QColor>
#include <QHash>
#include <QList>
#include <QPointer>

#include <functional>

/*!
  Synchronizes the horizontal (key) axis ranges of several QCustomPlot widgets and provides any
  number of draggable vertical rulers whose intersections with every graph are read out with a
  tracer and a text label.

  All bottom axes of all axis rects of the registered plots take part in the synchronization. A
  global key range limit can be configured; ranges are clamped to it while preserving their width
  (whole-range requests beyond a boundary are shifted back in, oversized requests fill the limit).

  Ruler interaction (hover cursor, left-button dragging of the nearest ruler) is handled by an
  event filter. Adding and removing rulers, keyboard nudging and any application specific policy
  stay with the host, which can query rulerAt() for hit testing.

  Ruler x-axis value labels are drawn in one designated plot only, see setLabelPlot().
*/
class QCP_LIB_DECL QCPSyncer : public QObject
{
  Q_OBJECT
public:
  explicit QCPSyncer(QObject *parent = nullptr);
  ~QCPSyncer() override;

  // plot registration:
  void addPlot(QCustomPlot *plot);
  void removePlot(QCustomPlot *plot);
  QList<QCustomPlot*> plots() const;
  //! Re-attaches ruler items and axis connections after axis rects and graphs were rebuilt.
  void rebuild();
  QList<QCPAxis*> syncedAxes() const;

  // key range limit:
  void setKeyRangeLimit(const QCPRange &range);
  void setKeyRangeLimit(double lower, double upper);
  void clearKeyRangeLimit();
  bool hasKeyRangeLimit() const;
  QCPRange keyRangeLimit() const;
  //! Broadcasts a range from outside (e.g. a scrollbar), applying the same clamp as user zoom/pan.
  void syncXRange(const QCPRange &range);

  // rulers:
  void addRuler(int id, double key);
  void removeRuler(int id);
  void clearRulers();
  bool hasRuler(int id) const;
  QList<int> rulerIds() const;
  double rulerKey(int id) const;
  void setRulerKey(int id, double key);
  void setRulersVisible(bool visible);
  bool rulersVisible() const;
  void setActiveRuler(int id);
  int activeRulerId() const;
  //! Default colors cycle through a nine color palette; override per ruler with setRulerColor.
  QColor rulerColor(int id) const;
  void setRulerColor(int id, const QColor &color);
  //! Returns the id of the ruler whose line is closest to \a pos (within the grab radius), or -1.
  int rulerAt(QCustomPlot *plot, const QPoint &pos) const;
  //! Returns the ruler registered under \a id, or nullptr if there is none.
  QCPRuler *ruler(int id) const;
  //! Returns all rulers in creation order.
  QList<QCPRuler*> rulers() const;
  //! Title drawn at the top of the label plot for the given ruler.
  void setRulerTitle(int id, const QString &title);
  QString rulerTitle(int id) const;
  //! Returns the id of the ruler whose title label contains \a pos, or -1.
  int rulerTitleAt(QCustomPlot *plot, const QPoint &pos) const;

  // readouts:
  void setReadoutEnabled(QCPGraph *graph, bool enabled);
  bool readoutEnabled(QCPGraph *graph) const;
  void rescanGraphs();
  //! Formats the interpolated graph value at a ruler key. Empty function restores the default.
  void setReadoutFormatter(std::function<QString(double)> formatter);

  // x value label:
  void setLabelPlot(QCustomPlot *plot);
  QCustomPlot *labelPlot() const;
  void setLabelAxisRect(QCPAxisRect *rect);
  QCPAxisRect *labelAxisRect() const;
  void setXLabelPlacement(QCPRuler::XLabelPlacement placement);
  QCPRuler::XLabelPlacement xLabelPlacement() const;
  //! Formats the ruler x value. Empty function restores the default (datetime tickers use their format).
  void setXLabelFormatter(std::function<QString(double)> formatter);

signals:
  void rulerKeyChanged(int id, double key);
  void rulerActivated(int id);
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
  QCPRange clampRange(const QCPRange &range) const;
  void applyRange(const QCPRange &range, QCPAxis *source);
  void updateDragCursor(QCustomPlot *plot, const QPoint &pos);
  QCPAxis *axisAt(QCustomPlot *plot, const QPoint &pos) const;
  void setRulerKeyInternal(int id, double key);
  void refreshAll();
  void queueReplotAll();
  static QColor defaultRulerColor(int id);

  QList<QCustomPlot*> mPlots;
  QList<QPointer<QCPAxis>> mConnectedAxes;
  bool mSyncing;
  bool mHasKeyLimit;
  QCPRange mKeyLimit;

  QHash<int, QCPRuler*> mRulers;
  QList<int> mRulerOrder;
  int mActiveRulerId;
  bool mRulersVisible;

  QPointer<QCustomPlot> mLabelPlot;
  QPointer<QCPAxisRect> mLabelAxisRect;
  QCPRuler::XLabelPlacement mXLabelPlacement;
  std::function<QString(double)> mReadoutFormatter;
  std::function<QString(double)> mXLabelFormatter;

  bool mDragging;
  int mDragRulerId;
  QPointer<QCustomPlot> mDragPlot;
  QPointer<QCPAxis> mDragAxis;
};

#endif // QCPSYNCER_H
