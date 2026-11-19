/* QCustomPlot 2.1.1 split source
   Copyright (C) 2011-2022 Emanuel Eichhammer
   Licensed under the GNU General Public License v3, see GPL.txt.
   Automatically split from the amalgamated qcustomplot.h/.cpp; keep code in sync. */

#ifndef QCPAXISBREAK_H
#define QCPAXISBREAK_H

#include "QCPGlobal.h"
#include "QCPAxisTicker.h"
#include "QCPRange.h"

#include <QHash>
#include <QList>
#include <QPen>
#include <QPointer>
#include <QString>
#include <QVector>

#include <functional>

class QCPAxis;
class QCPItemLine;

/*!
  Hides one or more key intervals by remapping the key coordinates piecewise and draws a slash
  marker at each hidden band.

  All keys passed to a QCustomPlot must be remapped with map() (or mapSeries()) beforehand. Within
  a real segment keys are only shifted, so segment lengths are preserved; each hidden interval
  occupies an extra band of breakWidth() mapped units, leaving room for the marker. Keys inside a
  hidden interval are dropped by mapSeries(), which also inserts a NaN point per band so that
  QCPGraph breaks its line there.

  attachAxis() installs a QCPAxisTickerBreak on the axis and keeps two slash marker items per band
  in sync with the axis range and the plot layout. Only horizontal axes are decorated.

  No interval is detected automatically: with an empty break list the object is inactive.
*/
class QCP_LIB_DECL QCPAxisBreak
{
public:
  QCPAxisBreak();
  ~QCPAxisBreak();

  // break definition:
  void setBreaks(const QVector<QCPRange> &breaks);
  QVector<QCPRange> breaks() const { return mBreaks; }
  bool isEmpty() const { return mBreaks.isEmpty(); }
  void setBreakWidth(double width);
  double breakWidth() const { return mBreakWidth; }

  // key mapping:
  double map(double key) const;
  QCPRange mapRange(const QCPRange &range) const;
  bool unmap(double mappedKey, double *key) const;
  bool isInBreak(double mappedKey) const;
  //! Maps sorted series data, dropping hidden points and inserting one NaN point per crossed band.
  void mapSeries(const QVector<double> &keys, const QVector<double> &values,
                 QVector<double> *mappedKeys, QVector<double> *mappedValues) const;

  //! Mapped position of the start/end of the hidden band belonging to break \a index.
  double mappedBandStart(int index) const;
  double mappedBandEnd(int index) const;

  // axis attachment:
  void attachAxis(QCPAxis *axis);
  void detachAxis(QCPAxis *axis);
  void detachAll();
  bool hasAxis(QCPAxis *axis) const;

  // marker appearance:
  void setMarkerVisible(bool visible);
  bool markerVisible() const { return mMarkerVisible; }
  void setMarkerPen(const QPen &pen);
  QPen markerPen() const { return mMarkerPen; }

private:
  Q_DISABLE_COPY(QCPAxisBreak)

  struct AxisAttachment
  {
    QPointer<QCPAxis> axis;
    QVector<QPointer<QCPItemLine> > lines;
    QVector<QMetaObject::Connection> connections;
    QSharedPointer<QCPAxisTicker> previousTicker;
  };

  void rebuildMapping();
  void ensureMarkerItems(QCPAxis *axis);
  void updateMarkers(QCPAxis *axis);
  void updateAllMarkers();

  QVector<QCPRange> mBreaks;
  QVector<double> mBandStarts;
  double mBreakWidth;
  bool mMarkerVisible;
  QPen mMarkerPen;
  QHash<QCPAxis*, AxisAttachment> mAxes;
};

/*!
  Ticker for axes whose key coordinates were remapped by a QCPAxisBreak.

  Major ticks are generated per real segment in original coordinates (so labels stay readable),
  then mapped back to axis positions; ticks falling into a hidden band are omitted, the two band
  boundaries are forced. The default inner ticker is QCPAxisTicker; set another one (e.g.
  QCPAxisTickerDateTime) to control tick style inside the segments. Custom inner tickers that
  format special labels can provide setBoundaryLabelFormatter for the forced boundary ticks.
*/
class QCP_LIB_DECL QCPAxisTickerBreak : public QCPAxisTicker
{
public:
  explicit QCPAxisTickerBreak(const QCPAxisBreak *axisBreak = nullptr);

  void setAxisBreak(const QCPAxisBreak *axisBreak);
  const QCPAxisBreak *axisBreak() const { return mAxisBreak; }
  void setInnerTicker(QSharedPointer<QCPAxisTicker> ticker);
  QSharedPointer<QCPAxisTicker> innerTicker() const { return mInnerTicker; }
  void setBoundaryLabelFormatter(std::function<QString(double)> formatter);
  std::function<QString(double)> boundaryLabelFormatter() const { return mBoundaryFormatter; }

protected:
  void generate(const QCPRange &range, const QLocale &locale, QChar formatChar, int precision,
                QVector<double> &ticks, QVector<double> *subTicks,
                QVector<QString> *tickLabels) override;

private:
  Q_DISABLE_COPY(QCPAxisTickerBreak)

  struct LabeledTick
  {
    double pos;
    QString label;
  };

  void generateInner(const QCPRange &range, const QLocale &locale, QChar formatChar, int precision,
                     QVector<double> &ticks, QVector<double> *subTicks,
                     QVector<QString> *tickLabels);
  void generateSegment(const QCPRange &mappedRange, const QLocale &locale, QChar formatChar,
                       int precision, QVector<LabeledTick> &ticks, QVector<double> &subTicks);
  void addBoundaryTick(double pos, double key, const QLocale &locale, QChar formatChar,
                       int precision, QVector<LabeledTick> &ticks);
  QString boundaryLabel(double key, const QLocale &locale, QChar formatChar, int precision);

  const QCPAxisBreak *mAxisBreak;
  QSharedPointer<QCPAxisTicker> mInnerTicker;
  std::function<QString(double)> mBoundaryFormatter;
};

#endif // QCPAXISBREAK_H
