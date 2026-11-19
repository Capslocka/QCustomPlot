/* QCustomPlot 2.1.1 split source
   Copyright (C) 2011-2022 Emanuel Eichhammer
   Licensed under the GNU General Public License v3, see GPL.txt.
   Automatically split from the amalgamated qcustomplot.h/.cpp; keep code in sync. */

#ifndef QCPRULER_H
#define QCPRULER_H

#include "QCPGlobal.h"
#include "QCustomPlot.h"
#include "QCPAxisRect.h"
#include "QCPGraph.h"
#include "QCPItemStraightLine.h"
#include "QCPItemTracer.h"
#include "QCPItemText.h"

#include <QColor>
#include <QHash>
#include <QList>
#include <QPointer>

#include <functional>

//! One draggable vertical ruler with per-graph readout items, spanning several QCustomPlot widgets.
class QCP_LIB_DECL QCPRuler
{
public:
  //! Where the ruler x value label is drawn relative to the label axis rect.
  enum XLabelPlacement { xlpBelowAxis ///< Below the bottom axis, outside the axis rect
                         ,xlpRectInside ///< Inside the axis rect, bottom edge, right-aligned at the ruler
                       };

  explicit QCPRuler(int id);
  ~QCPRuler();

  int id() const { return mId; }

  void setKey(double key);
  double key() const { return mKey; }
  void setVisible(bool visible);
  bool visible() const { return mVisible; }
  void setActive(bool active);
  bool active() const { return mActive; }
  void setColor(const QColor &color);
  QColor color() const { return mColor; }

  //! Title shown at the top of the label plot; empty by default (no item drawn).
  void setTitle(const QString &title);
  QString title() const { return mTitle; }
  //! True when \a pos (label plot coordinates) hits the drawn title item.
  bool titleHitTest(QCustomPlot *plot, const QPoint &pos) const;

  void setReadoutEnabled(QCPGraph *graph, bool enabled);
  bool readoutEnabled(QCPGraph *graph);
  void setReadoutFormatter(std::function<QString(double)> formatter);
  void setXLabelFormatter(std::function<QString(double)> formatter);
  void setLabelPlot(QCustomPlot *plot);
  void setLabelAxisRect(QCPAxisRect *rect);
  void setXLabelPlacement(XLabelPlacement placement);

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
    QPointer<QCPItemStraightLine> line;
    QHash<QCPGraph*, QPointer<QCPItemTracer> > tracers;
    QHash<QCPGraph*, QPointer<QCPItemText> > labels;
  };

  void createPlotItems(QCustomPlot *plot);
  void clearReadoutItems(QCustomPlot *plot);
  void clearPlotItems(QCustomPlot *plot);
  void ensureLabelItem();
  void discardLabelItem();
  void updatePlotVisuals(QCustomPlot *plot);
  void updateXLabel();
  void updateTitleLabel();
  QPen linePen() const;
  QString readoutText(QCPGraph *graph, bool *visible) const;
  QString xLabelText(double key, QCPAxis *axis) const;

  int mId;
  double mKey;
  bool mVisible;
  bool mActive;
  QColor mColor;
  QString mTitle;
  QList<QPointer<QCPGraph> > mDisabledGraphs;
  QHash<QCustomPlot*, PlotItems> mItems;
  QPointer<QCustomPlot> mLabelPlot;
  QPointer<QCPAxisRect> mLabelAxisRect;
  QPointer<QCPItemText> mXLabelItem;
  QPointer<QCPItemText> mTitleItem;
  XLabelPlacement mXLabelPlacement;
  std::function<QString(double)> mReadoutFormatter;
  std::function<QString(double)> mXLabelFormatter;
};

#endif // QCPRULER_H
