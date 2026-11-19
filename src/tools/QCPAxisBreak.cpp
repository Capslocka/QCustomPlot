/* QCustomPlot 2.1.1 split source
   Copyright (C) 2011-2022 Emanuel Eichhammer
   Licensed under the GNU General Public License v3, see GPL.txt.
   Automatically split from the amalgamated qcustomplot.h/.cpp; keep code in sync. */

#include "QCPAxisBreak.h"
#include "QCPAxis.h"
#include "QCPAxisRect.h"
#include "QCPItemLine.h"
#include "QCPItemPosition.h"

#include <QtGlobal>

#include <algorithm>
#include <cmath>
#include <limits>

// ---------------------------------------------------------------------------
// QCPAxisBreak
// ---------------------------------------------------------------------------

QCPAxisBreak::QCPAxisBreak() :
  mBreakWidth(0),
  mMarkerVisible(true),
  mMarkerPen(QPen(QColor(70, 70, 70), 1.2))
{
}

QCPAxisBreak::~QCPAxisBreak()
{
  detachAll();
}

void QCPAxisBreak::setBreaks(const QVector<QCPRange> &breaks)
{
  QVector<QCPRange> cleaned;
  cleaned.reserve(breaks.size());
  for (const QCPRange &range : breaks)
  {
    if (std::isfinite(range.lower) && std::isfinite(range.upper) && range.lower < range.upper)
      cleaned.append(range);
  }
  std::sort(cleaned.begin(), cleaned.end(), [](const QCPRange &a, const QCPRange &b) {
    return a.lower < b.lower;
  });

  QVector<QCPRange> merged;
  merged.reserve(cleaned.size());
  for (const QCPRange &range : cleaned)
  {
    if (!merged.isEmpty() && range.lower <= merged.last().upper)
    {
      if (range.upper > merged.last().upper)
        merged.last().upper = range.upper;
    } else
    {
      merged.append(range);
    }
  }

  mBreaks = merged;
  rebuildMapping();

  const QList<QCPAxis*> axes = mAxes.keys();
  for (QCPAxis *axis : axes)
    ensureMarkerItems(axis);
  updateAllMarkers();
}

void QCPAxisBreak::setBreakWidth(double width)
{
  const double clamped = std::isfinite(width) && width > 0 ? width : 0;
  if (qFuzzyCompare(clamped + 1.0, mBreakWidth + 1.0))
    return;
  mBreakWidth = clamped;
  rebuildMapping();
  updateAllMarkers();
}

void QCPAxisBreak::rebuildMapping()
{
  mBandStarts.clear();
  mBandStarts.reserve(mBreaks.size());
  double shift = 0;
  for (const QCPRange &range : mBreaks)
  {
    mBandStarts.append(range.lower + shift);
    shift += mBreakWidth - (range.upper - range.lower);
  }
}

double QCPAxisBreak::map(double key) const
{
  double shift = 0;
  for (const QCPRange &range : mBreaks)
  {
    if (key < range.lower)
      return key + shift;
    if (key > range.upper)
    {
      shift += mBreakWidth - (range.upper - range.lower);
      continue;
    }
    // Time bases are often accumulated products (e.g. i*dt), so a sample nominally on a
    // boundary can land a few ulps inside it. Treat it as on the boundary instead of
    // hiding a visible sample.
    if (qFuzzyCompare(key, range.lower))
      return range.lower + shift;
    if (qFuzzyCompare(key, range.upper))
      return range.upper + shift + mBreakWidth - (range.upper - range.lower);
    return range.lower + shift + mBreakWidth * 0.5; // hidden keys collapse to the band centre
  }
  return key + shift;
}

QCPRange QCPAxisBreak::mapRange(const QCPRange &range) const
{
  return QCPRange(map(range.lower), map(range.upper));
}

bool QCPAxisBreak::unmap(double mappedKey, double *key) const
{
  double shift = 0;
  for (int i = 0; i < mBreaks.size(); ++i)
  {
    const QCPRange &range = mBreaks.at(i);
    const double start = mBandStarts.at(i);
    const double end = start + mBreakWidth;
    if (mappedKey < start)
    {
      if (key)
        *key = mappedKey - shift;
      return true;
    }
    if (mappedKey > end)
    {
      shift += mBreakWidth - (range.upper - range.lower);
      continue;
    }
    if (qFuzzyCompare(mappedKey, start))
    {
      if (key)
        *key = range.lower;
      return true;
    }
    if (qFuzzyCompare(mappedKey, end))
    {
      if (key)
        *key = range.upper;
      return true;
    }
    return false;
  }
  if (key)
    *key = mappedKey - shift;
  return true;
}

bool QCPAxisBreak::isInBreak(double mappedKey) const
{
  if (mBreakWidth <= 0)
    return false;
  for (int i = 0; i < mBreaks.size(); ++i)
  {
    const double start = mBandStarts.at(i);
    const double end = start + mBreakWidth;
    if (mappedKey < start)
    {
      if (qFuzzyCompare(mappedKey, start))
        continue;
      return false;
    }
    if (mappedKey > end)
      continue;
    if (qFuzzyCompare(mappedKey, start) || qFuzzyCompare(mappedKey, end))
      continue;
    return true;
  }
  return false;
}

double QCPAxisBreak::mappedBandStart(int index) const
{
  return mBandStarts.value(index, 0);
}

double QCPAxisBreak::mappedBandEnd(int index) const
{
  return mappedBandStart(index) + mBreakWidth;
}

void QCPAxisBreak::mapSeries(const QVector<double> &keys, const QVector<double> &values,
                             QVector<double> *mappedKeys, QVector<double> *mappedValues) const
{
  if (!mappedKeys || !mappedValues)
    return;
  mappedKeys->clear();
  mappedValues->clear();
  if (keys.isEmpty() || keys.size() != values.size())
    return;

  const double firstKey = keys.first();
  const double lastKey = keys.last();
  QVector<bool> needsNaN(mBreaks.size(), false);
  for (int i = 0; i < mBreaks.size(); ++i)
  {
    const QCPRange &range = mBreaks.at(i);
    // Boundary samples belong to the adjacent visible segment, so equal or fuzzy-equal
    // counts as on the boundary; otherwise a series ending on the bound stays connected.
    needsNaN[i] = (firstKey <= range.lower || qFuzzyCompare(firstKey, range.lower))
        && (lastKey >= range.upper || qFuzzyCompare(lastKey, range.upper));
  }

  mappedKeys->reserve(keys.size() + mBreaks.size());
  mappedValues->reserve(keys.size() + mBreaks.size());

  int bandIndex = 0;
  QVector<bool> emitted(mBreaks.size(), false);
  for (int i = 0; i < keys.size(); ++i)
  {
    const double key = keys.at(i);
    // Flush before appending any key that map() treats as the band end: the NaN band-centre
    // key is smaller, and appending it later would leave the key sequence unsorted.
    while (bandIndex < mBreaks.size())
    {
      const QCPRange &range = mBreaks.at(bandIndex);
      if (key < range.upper && !qFuzzyCompare(key, range.upper))
        break;
      if (needsNaN.at(bandIndex) && !emitted.at(bandIndex))
      {
        mappedKeys->append(mBandStarts.at(bandIndex) + mBreakWidth * 0.5);
        mappedValues->append(qQNaN());
        emitted[bandIndex] = true;
      }
      ++bandIndex;
    }
    if (bandIndex < mBreaks.size())
    {
      const QCPRange &range = mBreaks.at(bandIndex);
      if (key > range.lower && !qFuzzyCompare(key, range.lower)
          && key < range.upper && !qFuzzyCompare(key, range.upper))
        continue; // point strictly inside a hidden interval: drop it
    }
    mappedKeys->append(map(key));
    mappedValues->append(values.at(i));
  }
}

void QCPAxisBreak::attachAxis(QCPAxis *axis)
{
  if (!axis || mAxes.contains(axis))
    return;

  AxisAttachment attachment;
  attachment.axis = axis;
  attachment.previousTicker = axis->ticker();
  axis->setTicker(QSharedPointer<QCPAxisTickerBreak>(new QCPAxisTickerBreak(this)));
  mAxes.insert(axis, attachment);

  ensureMarkerItems(axis);

  if (QCustomPlot *plot = axis->parentPlot())
  {
    mAxes[axis].connections.append(
        QObject::connect(axis, static_cast<void (QCPAxis::*)(const QCPRange&)>(&QCPAxis::rangeChanged),
                         axis, [this, axis](const QCPRange&) { updateMarkers(axis); }));
    mAxes[axis].connections.append(
        QObject::connect(plot, &QCustomPlot::afterLayout, axis, [this, axis] { updateMarkers(axis); }));
  }
  updateMarkers(axis);
}

void QCPAxisBreak::detachAxis(QCPAxis *axis)
{
  auto it = mAxes.find(axis);
  if (it == mAxes.end())
    return;

  for (const QMetaObject::Connection &connection : it->connections)
    QObject::disconnect(connection);
  for (const QPointer<QCPItemLine> &line : it->lines)
  {
    if (line)
    {
      if (QCustomPlot *plot = line->parentPlot())
        plot->removeItem(line);
    }
  }
  if (it->axis && it->previousTicker)
    it->axis->setTicker(it->previousTicker);
  mAxes.erase(it);
}

void QCPAxisBreak::detachAll()
{
  const QList<QCPAxis*> axes = mAxes.keys();
  for (QCPAxis *axis : axes)
    detachAxis(axis);
}

bool QCPAxisBreak::hasAxis(QCPAxis *axis) const
{
  return mAxes.contains(axis);
}

void QCPAxisBreak::setMarkerVisible(bool visible)
{
  if (mMarkerVisible == visible)
    return;
  mMarkerVisible = visible;
  updateAllMarkers();
}

void QCPAxisBreak::setMarkerPen(const QPen &pen)
{
  mMarkerPen = pen;
  updateAllMarkers();
}

void QCPAxisBreak::ensureMarkerItems(QCPAxis *axis)
{
  auto it = mAxes.find(axis);
  if (it == mAxes.end())
    return;
  QCustomPlot *plot = axis ? axis->parentPlot() : nullptr;
  if (!plot)
    return;

  const int needed = mBreaks.size() * 2;
  while (it->lines.size() < needed)
  {
    QCPItemLine *line = new QCPItemLine(plot);
    line->setLayer("overlay");
    line->setClipToAxisRect(false);
    line->start->setType(QCPItemPosition::ptAbsolute);
    line->end->setType(QCPItemPosition::ptAbsolute);
    line->setVisible(false);
    it->lines.append(line);
  }
  while (it->lines.size() > needed)
  {
    QPointer<QCPItemLine> line = it->lines.last();
    it->lines.removeLast();
    if (line)
      plot->removeItem(line);
  }
}

void QCPAxisBreak::updateMarkers(QCPAxis *axis)
{
  auto it = mAxes.find(axis);
  if (it == mAxes.end())
    return;

  const bool horizontal = axis && axis->orientation() == Qt::Horizontal;
  QCPAxisRect *axisRect = axis ? axis->axisRect() : nullptr;
  const bool drawable = horizontal && axisRect && axis->visible() && !mBreaks.isEmpty();
  const double y = drawable
      ? (axis->axisType() == QCPAxis::atTop ? axisRect->rect().top() : axisRect->rect().bottom())
      : 0;

  for (int i = 0; i < mBreaks.size(); ++i)
  {
    const double center = mappedBandStart(i) + mBreakWidth * 0.5;
    const double pixel = drawable ? axis->coordToPixel(center) : -1;
    const bool visible = drawable && mMarkerVisible && pixel >= axisRect->rect().left() - 6
        && pixel <= axisRect->rect().right() + 6;

    for (int slash = 0; slash < 2; ++slash)
    {
      QPointer<QCPItemLine> line = it->lines.value(i * 2 + slash);
      if (!line)
        continue;
      line->setVisible(visible);
      if (!visible)
        continue;
      const double cx = pixel + (slash == 0 ? -3.0 : 3.0);
      line->start->setCoords(cx - 3.0, y + 5.0);
      line->end->setCoords(cx + 3.0, y - 5.0);
      line->setPen(mMarkerPen);
    }
  }
}

void QCPAxisBreak::updateAllMarkers()
{
  const QList<QCPAxis*> axes = mAxes.keys();
  for (QCPAxis *axis : axes)
    updateMarkers(axis);
}

// ---------------------------------------------------------------------------
// QCPAxisTickerBreak
// ---------------------------------------------------------------------------

QCPAxisTickerBreak::QCPAxisTickerBreak(const QCPAxisBreak *axisBreak) :
  mAxisBreak(axisBreak),
  mInnerTicker(new QCPAxisTicker)
{
}

void QCPAxisTickerBreak::setAxisBreak(const QCPAxisBreak *axisBreak)
{
  mAxisBreak = axisBreak;
}

void QCPAxisTickerBreak::setInnerTicker(QSharedPointer<QCPAxisTicker> ticker)
{
  mInnerTicker = ticker ? ticker : QSharedPointer<QCPAxisTicker>(new QCPAxisTicker);
}

void QCPAxisTickerBreak::setBoundaryLabelFormatter(std::function<QString(double)> formatter)
{
  mBoundaryFormatter = formatter;
}

void QCPAxisTickerBreak::generate(const QCPRange &range, const QLocale &locale, QChar formatChar,
                                  int precision, QVector<double> &ticks, QVector<double> *subTicks,
                                  QVector<QString> *tickLabels)
{
  if (!mAxisBreak || mAxisBreak->isEmpty() || !mInnerTicker)
  {
    generateInner(range, locale, formatChar, precision, ticks, subTicks, tickLabels);
    return;
  }

  const QVector<QCPRange> breaks = mAxisBreak->breaks();
  const double bandWidth = mAxisBreak->breakWidth();

  QVector<LabeledTick> collectedTicks;
  QVector<double> collectedSubTicks;
  double cursor = range.lower;

  for (int i = 0; i < breaks.size(); ++i)
  {
    const double bandStart = mAxisBreak->mappedBandStart(i);
    const double bandEnd = bandStart + bandWidth;
    if (bandEnd < range.lower)
      continue;
    if (bandStart > range.upper)
      break;

    const double segmentEnd = qMin(bandStart, range.upper);
    if (segmentEnd > cursor)
      generateSegment(QCPRange(cursor, segmentEnd), locale, formatChar, precision, collectedTicks,
                      collectedSubTicks);

    if (bandStart >= range.lower && bandStart <= range.upper)
      addBoundaryTick(bandStart, breaks.at(i).lower, locale, formatChar, precision, collectedTicks);
    if (bandEnd >= range.lower && bandEnd <= range.upper)
      addBoundaryTick(bandEnd, breaks.at(i).upper, locale, formatChar, precision, collectedTicks);

    cursor = qMax(cursor, bandEnd);
    if (bandEnd > range.upper)
    {
      cursor = range.upper;
      break;
    }
  }
  if (cursor < range.upper)
    generateSegment(QCPRange(cursor, range.upper), locale, formatChar, precision, collectedTicks,
                    collectedSubTicks);

  std::stable_sort(collectedTicks.begin(), collectedTicks.end(),
                   [](const LabeledTick &a, const LabeledTick &b) { return a.pos < b.pos; });

  ticks.clear();
  if (tickLabels)
    tickLabels->clear();
  double lastPosition = -std::numeric_limits<double>::infinity();
  for (const LabeledTick &tick : collectedTicks)
  {
    if (qFuzzyCompare(tick.pos, lastPosition))
      continue;
    lastPosition = tick.pos;
    ticks.append(tick.pos);
    if (tickLabels)
      tickLabels->append(tick.label);
  }

  if (subTicks)
  {
    std::sort(collectedSubTicks.begin(), collectedSubTicks.end());
    subTicks->clear();
    double lastSubPosition = -std::numeric_limits<double>::infinity();
    for (double position : collectedSubTicks)
    {
      if (position < range.lower || position > range.upper)
        continue;
      if (qFuzzyCompare(position, lastSubPosition))
        continue;
      lastSubPosition = position;
      subTicks->append(position);
    }
  }
}

void QCPAxisTickerBreak::generateInner(const QCPRange &range, const QLocale &locale,
                                       QChar formatChar, int precision, QVector<double> &ticks,
                                       QVector<double> *subTicks, QVector<QString> *tickLabels)
{
  if (mInnerTicker)
    mInnerTicker->generate(range, locale, formatChar, precision, ticks, subTicks, tickLabels);
  else
    QCPAxisTicker::generate(range, locale, formatChar, precision, ticks, subTicks, tickLabels);
}

void QCPAxisTickerBreak::generateSegment(const QCPRange &mappedRange, const QLocale &locale,
                                         QChar formatChar, int precision,
                                         QVector<LabeledTick> &ticks, QVector<double> &subTicks)
{
  double originalLower = 0;
  double originalUpper = 0;
  if (!mAxisBreak->unmap(mappedRange.lower, &originalLower)
      || !mAxisBreak->unmap(mappedRange.upper, &originalUpper))
  {
    return;
  }
  if (!(originalUpper > originalLower))
    return;

  QVector<double> innerTicks;
  QVector<double> innerSubTicks;
  QVector<QString> innerLabels;
  mInnerTicker->generate(QCPRange(originalLower, originalUpper), locale, formatChar, precision,
                         innerTicks, &innerSubTicks, &innerLabels);

  for (int i = 0; i < innerTicks.size(); ++i)
  {
    const double position = mAxisBreak->map(innerTicks.at(i));
    if ((position < mappedRange.lower && !qFuzzyCompare(position, mappedRange.lower))
        || (position > mappedRange.upper && !qFuzzyCompare(position, mappedRange.upper)))
      continue;
    LabeledTick tick;
    tick.pos = position;
    tick.label = innerLabels.value(i);
    ticks.append(tick);
  }
  for (double innerSubTick : innerSubTicks)
  {
    const double position = mAxisBreak->map(innerSubTick);
    if ((position < mappedRange.lower && !qFuzzyCompare(position, mappedRange.lower))
        || (position > mappedRange.upper && !qFuzzyCompare(position, mappedRange.upper)))
      continue;
    subTicks.append(position);
  }
}

void QCPAxisTickerBreak::addBoundaryTick(double pos, double key, const QLocale &locale,
                                         QChar formatChar, int precision,
                                         QVector<LabeledTick> &ticks)
{
  for (const LabeledTick &existing : ticks)
  {
    if (qFuzzyCompare(existing.pos, pos))
      return;
  }
  LabeledTick tick;
  tick.pos = pos;
  tick.label = boundaryLabel(key, locale, formatChar, precision);
  ticks.append(tick);
}

QString QCPAxisTickerBreak::boundaryLabel(double key, const QLocale &locale, QChar formatChar,
                                          int precision)
{
  if (mBoundaryFormatter)
    return mBoundaryFormatter(key);
  return createLabelVector(QVector<double>(1, key), locale, formatChar, precision).value(0);
}
