/* QCustomPlot 2.1.1 split source
   Copyright (C) 2011-2022 Emanuel Eichhammer
   Licensed under the GNU General Public License v3, see GPL.txt.
   Automatically split from the amalgamated qcustomplot.h/.cpp; keep code in sync. */

#include "QCPSyncer.h"

#include <QEvent>
#include <QMouseEvent>
#include <QtGlobal>

#include <algorithm>

// ---------------------------------------------------------------------------
// QCPSyncer
// ---------------------------------------------------------------------------

QCPSyncer::QCPSyncer(QObject *parent) :
  QObject(parent),
  mSyncing(false),
  mHasKeyLimit(false),
  mKeyLimit(0, 0),
  mActiveRulerId(-1),
  mRulersVisible(true),
  mXLabelPlacement(QCPRuler::xlpBelowAxis),
  mDragging(false),
  mDragRulerId(-1)
{
}

QCPSyncer::~QCPSyncer()
{
  const QList<QCustomPlot*> plots = mPlots;
  for (QCustomPlot *plot : plots)
  {
    if (plot)
    {
      plot->removeEventFilter(this);
      for (QCPRuler *ruler : mRulers)
        ruler->detachPlot(plot);
    }
  }
  qDeleteAll(mRulers);
  mRulers.clear();
}

QColor QCPSyncer::defaultRulerColor(int id)
{
  static const QColor palette[] = {
    QColor(QStringLiteral("#E53935")), // red
    QColor(QStringLiteral("#1E88E5")), // blue
    QColor(QStringLiteral("#43A047")), // green
    QColor(QStringLiteral("#FB8C00")), // orange
    QColor(QStringLiteral("#8E24AA")), // purple
    QColor(QStringLiteral("#00ACC1")), // cyan
    QColor(QStringLiteral("#D81B60")), // magenta
    QColor(QStringLiteral("#FDD835")), // yellow
    QColor(QStringLiteral("#757575"))  // gray
  };
  const int count = int(sizeof(palette) / sizeof(palette[0]));
  int index = (id - 1) % count;
  if (index < 0)
    index += count;
  return palette[index];
}

void QCPSyncer::addPlot(QCustomPlot *plot)
{
  if (!plot || mPlots.contains(plot))
    return;

  QCPRange alignRange;
  bool hasAlignRange = false;
  {
    const QList<QCPAxis*> axes = effectiveSyncedAxes();
    if (!axes.isEmpty())
    {
      alignRange = axes.first()->range();
      hasAlignRange = true;
    }
  }

  mPlots.append(plot);
  plot->installEventFilter(this);
  plot->setMouseTracking(true);
  connect(plot, &QCustomPlot::afterLayout, this, [this, plot]() {
    // afterLayout runs at the end of QCustomPlot::updateLayout() and before the layers
    // are drawn into their paint buffers, and it already sees the fresh axis rect
    // geometry. Refreshing here (instead of beforeReplot) keeps ruler items in sync on
    // the very first replot after graphs are rebuilt or the plot is shown.
    for (QCPRuler *ruler : mRulers)
      ruler->refreshPlot(plot);
  });
  connect(plot, &QObject::destroyed, this, [this, plot]() {
    mPlots.removeAll(plot);
    mConnectedAxes.clear();
    for (QCPRuler *ruler : mRulers)
      ruler->forgetPlot(plot);
    if (mLabelPlot == plot)
      mLabelPlot = mPlots.isEmpty() ? nullptr : mPlots.first();
    if (mDragPlot == plot)
    {
      mDragging = false;
      mDragPlot = nullptr;
      mDragAxis = nullptr;
      mDragRulerId = -1;
    }
    rebuildConnections();
    if (mLabelPlot)
      setLabelPlot(mLabelPlot);
    refreshAll();
  });

  for (QCPRuler *ruler : mRulers)
    ruler->attachPlot(plot);
  if (!mLabelPlot)
    mLabelPlot = plot;
  if (mLabelPlot)
    setLabelPlot(mLabelPlot);
  if (mLabelAxisRect)
  {
    for (QCPRuler *ruler : mRulers)
      ruler->setLabelAxisRect(mLabelAxisRect);
  }

  rebuildConnections();
  if (hasAlignRange)
  {
    mSyncing = true;
    const QList<QCPAxis*> axes = effectiveSyncedAxes();
    for (QCPAxis *axis : axes)
    {
      if (axis->parentPlot() == plot)
        axis->setRange(alignRange);
    }
    mSyncing = false;
  }
  refreshAll();
  queueReplotAll();
}

void QCPSyncer::removePlot(QCustomPlot *plot)
{
  if (!plot || !mPlots.contains(plot))
    return;
  plot->removeEventFilter(this);
  for (QCPRuler *ruler : mRulers)
    ruler->detachPlot(plot);
  mPlots.removeAll(plot);
  if (mLabelPlot == plot)
  {
    mLabelPlot = mPlots.isEmpty() ? nullptr : mPlots.first();
    if (mLabelPlot)
      setLabelPlot(mLabelPlot);
  }
  if (mDragPlot == plot)
  {
    mDragging = false;
    mDragPlot = nullptr;
    mDragAxis = nullptr;
    mDragRulerId = -1;
  }
  rebuildConnections();
  refreshAll();
  queueReplotAll();
}

QList<QCustomPlot*> QCPSyncer::plots() const
{
  return mPlots;
}

void QCPSyncer::rebuild()
{
  const QList<QCustomPlot*> plots = mPlots;
  for (QCPRuler *ruler : mRulers)
  {
    for (QCustomPlot *plot : plots)
    {
      if (plot)
        ruler->detachPlot(plot);
    }
  }
  rebuildConnections();
  for (QCustomPlot *plot : plots)
  {
    if (!plot)
      continue;
    for (QCPRuler *ruler : mRulers)
      ruler->attachPlot(plot);
  }
  for (QCPRuler *ruler : mRulers)
  {
    if (mLabelPlot)
      ruler->setLabelPlot(mLabelPlot);
    ruler->setLabelAxisRect(mLabelAxisRect);
    ruler->setXLabelPlacement(mXLabelPlacement);
    ruler->refresh();
  }
  queueReplotAll();
}

QList<QCPAxis*> QCPSyncer::syncedAxes() const
{
  return effectiveSyncedAxes();
}

void QCPSyncer::setKeyRangeLimit(const QCPRange &range)
{
  mHasKeyLimit = true;
  mKeyLimit = QCPRange(qMin(range.lower, range.upper), qMax(range.lower, range.upper));
  const QList<QCPAxis*> axes = effectiveSyncedAxes();
  if (!axes.isEmpty())
    applyRange(axes.first()->range(), nullptr);
}

void QCPSyncer::setKeyRangeLimit(double lower, double upper)
{
  setKeyRangeLimit(QCPRange(lower, upper));
}

void QCPSyncer::clearKeyRangeLimit()
{
  mHasKeyLimit = false;
}

bool QCPSyncer::hasKeyRangeLimit() const
{
  return mHasKeyLimit;
}

QCPRange QCPSyncer::keyRangeLimit() const
{
  return mKeyLimit;
}

void QCPSyncer::syncXRange(const QCPRange &range)
{
  if (mSyncing)
    return;
  applyRange(range, nullptr);
}

void QCPSyncer::addRuler(int id, double key)
{
  if (mRulers.contains(id))
  {
    setRulerKey(id, key);
    return;
  }

  QCPRuler *ruler = new QCPRuler(id);
  if (mHasKeyLimit)
    key = clampRange(QCPRange(key, key)).lower;
  ruler->setKey(key);
  ruler->setColor(defaultRulerColor(id));
  ruler->setVisible(mRulersVisible);
  ruler->setReadoutFormatter(mReadoutFormatter);
  ruler->setXLabelFormatter(mXLabelFormatter);
  ruler->setXLabelPlacement(mXLabelPlacement);
  for (QCustomPlot *plot : mPlots)
  {
    if (plot)
      ruler->attachPlot(plot);
  }
  if (!mLabelPlot && !mPlots.isEmpty())
    mLabelPlot = mPlots.first();
  if (mLabelPlot)
    ruler->setLabelPlot(mLabelPlot);
  if (mLabelAxisRect)
    ruler->setLabelAxisRect(mLabelAxisRect);

  mRulers.insert(id, ruler);
  mRulerOrder.append(id);
  if (mActiveRulerId < 0)
  {
    mActiveRulerId = id;
    ruler->setActive(true);
  }
  ruler->refresh();
  queueReplotAll();
}

void QCPSyncer::removeRuler(int id)
{
  QCPRuler *ruler = mRulers.take(id);
  if (!ruler)
    return;
  mRulerOrder.removeAll(id);
  delete ruler;
  if (mActiveRulerId == id)
  {
    mActiveRulerId = mRulerOrder.isEmpty() ? -1 : mRulerOrder.last();
    if (mActiveRulerId >= 0)
      setActiveRuler(mActiveRulerId);
  }
  refreshAll();
  queueReplotAll();
}

void QCPSyncer::clearRulers()
{
  qDeleteAll(mRulers);
  mRulers.clear();
  mRulerOrder.clear();
  mActiveRulerId = -1;
  queueReplotAll();
}

bool QCPSyncer::hasRuler(int id) const
{
  return mRulers.contains(id);
}

QList<int> QCPSyncer::rulerIds() const
{
  return mRulerOrder;
}

double QCPSyncer::rulerKey(int id) const
{
  QCPRuler *ruler = mRulers.value(id);
  return ruler ? ruler->key() : 0.0;
}

void QCPSyncer::setRulerKey(int id, double key)
{
  if (!mRulers.contains(id))
    return;
  setRulerKeyInternal(id, key);
}

void QCPSyncer::setRulersVisible(bool visible)
{
  if (mRulersVisible == visible)
    return;
  mRulersVisible = visible;
  for (QCPRuler *ruler : mRulers)
    ruler->setVisible(visible);
  refreshAll();
  queueReplotAll();
}

bool QCPSyncer::rulersVisible() const
{
  return mRulersVisible;
}

void QCPSyncer::setActiveRuler(int id)
{
  mActiveRulerId = id;
  for (QCPRuler *ruler : mRulers)
    ruler->setActive(ruler->id() == id);
  refreshAll();
  queueReplotAll();
}

int QCPSyncer::activeRulerId() const
{
  if (mRulers.contains(mActiveRulerId))
    return mActiveRulerId;
  return mRulerOrder.isEmpty() ? -1 : mRulerOrder.last();
}

QColor QCPSyncer::rulerColor(int id) const
{
  QCPRuler *ruler = mRulers.value(id);
  return ruler ? ruler->color() : defaultRulerColor(id);
}

void QCPSyncer::setRulerColor(int id, const QColor &color)
{
  QCPRuler *ruler = mRulers.value(id);
  if (!ruler)
    return;
  ruler->setColor(color);
  refreshAll();
  queueReplotAll();
}

int QCPSyncer::rulerAt(QCustomPlot *plot, const QPoint &pos) const
{
  QCPAxis *axis = axisAt(plot, pos);
  if (!axis)
    return -1;
  int bestId = -1;
  double bestDistance = 6; // rulers within 6 px of the cursor are grabbable
  for (int id : mRulerOrder)
  {
    QCPRuler *ruler = mRulers.value(id);
    if (!ruler)
      continue;
    const double distance = qAbs(axis->coordToPixel(ruler->key()) - pos.x());
    if (distance <= bestDistance)
    {
      bestDistance = distance;
      bestId = id;
    }
  }
  return bestId;
}

QCPRuler *QCPSyncer::ruler(int id) const
{
  return mRulers.value(id, nullptr);
}

QList<QCPRuler*> QCPSyncer::rulers() const
{
  QList<QCPRuler*> result;
  result.reserve(mRulerOrder.size());
  for (int id : mRulerOrder)
  {
    QCPRuler *r = mRulers.value(id, nullptr);
    if (r)
      result.append(r);
  }
  return result;
}

void QCPSyncer::setRulerTitle(int id, const QString &title)
{
  QCPRuler *r = mRulers.value(id);
  if (!r)
    return;
  r->setTitle(title);
  r->refresh();
  queueReplotAll();
}

QString QCPSyncer::rulerTitle(int id) const
{
  QCPRuler *r = mRulers.value(id);
  return r ? r->title() : QString();
}

int QCPSyncer::rulerTitleAt(QCustomPlot *plot, const QPoint &pos) const
{
  for (int id : mRulerOrder)
  {
    QCPRuler *r = mRulers.value(id);
    if (r && r->titleHitTest(plot, pos))
      return id;
  }
  return -1;
}

void QCPSyncer::setReadoutEnabled(QCPGraph *graph, bool enabled)
{
  for (QCPRuler *ruler : mRulers)
    ruler->setReadoutEnabled(graph, enabled);
  refreshAll();
  queueReplotAll();
}

bool QCPSyncer::readoutEnabled(QCPGraph *graph) const
{
  for (QCPRuler *ruler : mRulers)
  {
    if (ruler->readoutEnabled(graph))
      return true;
  }
  return mRulers.isEmpty();
}

void QCPSyncer::rescanGraphs()
{
  for (QCPRuler *ruler : mRulers)
    ruler->rescanAll();
  refreshAll();
  queueReplotAll();
}

void QCPSyncer::setLabelPlot(QCustomPlot *plot)
{
  if (!plot || !mPlots.contains(plot))
    return;
  mLabelPlot = plot;
  for (QCPRuler *ruler : mRulers)
    ruler->setLabelPlot(plot);
  refreshAll();
  queueReplotAll();
}

QCustomPlot *QCPSyncer::labelPlot() const
{
  return mLabelPlot;
}

void QCPSyncer::setLabelAxisRect(QCPAxisRect *rect)
{
  mLabelAxisRect = rect;
  for (QCPRuler *ruler : mRulers)
    ruler->setLabelAxisRect(rect);
  refreshAll();
  queueReplotAll();
}

QCPAxisRect *QCPSyncer::labelAxisRect() const
{
  if (mLabelAxisRect)
    return mLabelAxisRect;
  return mLabelPlot ? mLabelPlot->axisRect() : nullptr;
}

void QCPSyncer::setXLabelPlacement(QCPRuler::XLabelPlacement placement)
{
  mXLabelPlacement = placement;
  for (QCPRuler *ruler : mRulers)
    ruler->setXLabelPlacement(placement);
  refreshAll();
  queueReplotAll();
}

QCPRuler::XLabelPlacement QCPSyncer::xLabelPlacement() const
{
  return mXLabelPlacement;
}

void QCPSyncer::setReadoutFormatter(std::function<QString(double)> formatter)
{
  mReadoutFormatter = formatter;
  for (QCPRuler *ruler : mRulers)
    ruler->setReadoutFormatter(formatter);
  refreshAll();
  queueReplotAll();
}

void QCPSyncer::setXLabelFormatter(std::function<QString(double)> formatter)
{
  mXLabelFormatter = formatter;
  for (QCPRuler *ruler : mRulers)
    ruler->setXLabelFormatter(formatter);
  refreshAll();
  queueReplotAll();
}

bool QCPSyncer::eventFilter(QObject *watched, QEvent *event)
{
  QCustomPlot *plot = nullptr;
  for (QCustomPlot *candidate : mPlots)
  {
    if (candidate && candidate == watched)
    {
      plot = candidate;
      break;
    }
  }
  if (!plot)
    return QObject::eventFilter(watched, event);

  switch (event->type())
  {
    case QEvent::MouseButtonPress:
    {
      QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
      if (mouseEvent->button() != Qt::LeftButton)
        break;
      const QPoint pos = mouseEvent->pos();
      // Title clicks stay with the host (rename dialog); only bare line hits start a drag.
      if (rulerTitleAt(plot, pos) >= 0)
        break;
      const int id = rulerAt(plot, pos);
      if (id < 0)
        break;
      QCPAxis *axis = axisAt(plot, pos);
      if (!axis)
        break;
      mDragging = true;
      mDragRulerId = id;
      mDragPlot = plot;
      mDragAxis = axis;
      setActiveRuler(id);
      emit rulerActivated(id);
      plot->setCursor(Qt::SplitHCursor);
      double key = axis->pixelToCoord(pos.x());
      key = qBound(qMin(axis->range().lower, axis->range().upper),
                   key, qMax(axis->range().lower, axis->range().upper));
      setRulerKeyInternal(id, key);
      return true;
    }
    case QEvent::MouseMove:
    {
      QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
      const QPoint pos = mouseEvent->pos();
      if (mDragging && mDragPlot == plot && mDragAxis)
      {
        double key = mDragAxis->pixelToCoord(pos.x());
        key = qBound(qMin(mDragAxis->range().lower, mDragAxis->range().upper),
                     key, qMax(mDragAxis->range().lower, mDragAxis->range().upper));
        setRulerKeyInternal(mDragRulerId, key);
        return true;
      }
      updateDragCursor(plot, pos);
      break;
    }
    case QEvent::MouseButtonRelease:
    {
      QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
      if (mDragging && mouseEvent->button() == Qt::LeftButton)
      {
        mDragging = false;
        mDragRulerId = -1;
        mDragPlot = nullptr;
        mDragAxis = nullptr;
        updateDragCursor(plot, mouseEvent->pos());
        queueReplotAll();
        return true;
      }
      break;
    }
    case QEvent::Leave:
      if (!mDragging)
        plot->unsetCursor();
      break;
    default:
      break;
  }
  return QObject::eventFilter(watched, event);
}

void QCPSyncer::onAxisRangeChanged(const QCPRange &range)
{
  if (mSyncing)
    return;
  QCPAxis *source = qobject_cast<QCPAxis*>(sender());
  applyRange(range, source);
}

QCPRange QCPSyncer::clampRange(const QCPRange &range) const
{
  if (!mHasKeyLimit)
    return range;
  const double limitLower = mKeyLimit.lower;
  const double limitUpper = mKeyLimit.upper;
  const double lower = qMin(range.lower, range.upper);
  const double upper = qMax(range.lower, range.upper);
  const double width = upper - lower;
  if (width >= limitUpper - limitLower)
    return QCPRange(limitLower, limitUpper);
  double newLower = lower;
  if (newLower < limitLower)
    newLower = limitLower;
  if (newLower + width > limitUpper)
    newLower = limitUpper - width;
  return QCPRange(newLower, newLower + width);
}

void QCPSyncer::applyRange(const QCPRange &range, QCPAxis *source)
{
  Q_UNUSED(source)
  const QCPRange target = clampRange(range);
  mSyncing = true;
  const QList<QCPAxis*> axes = effectiveSyncedAxes();
  for (QCPAxis *axis : axes)
  {
    if (!axis)
      continue;
    const QCPRange current = axis->range();
    if (!qFuzzyCompare(current.lower, target.lower) || !qFuzzyCompare(current.upper, target.upper))
      axis->setRange(target);
  }
  mSyncing = false;
  emit syncedRangeChanged(target);
  refreshAll();
  queueReplotAll();
}

QList<QCPAxis*> QCPSyncer::effectiveSyncedAxes() const
{
  QList<QCPAxis*> result;
  for (QCustomPlot *plot : mPlots)
  {
    if (!plot)
      continue;
    for (int i = 0; i < plot->axisRectCount(); ++i)
    {
      QCPAxisRect *rect = plot->axisRect(i);
      QCPAxis *axis = rect ? rect->axis(QCPAxis::atBottom) : nullptr;
      if (axis && !result.contains(axis))
        result.append(axis);
    }
  }
  return result;
}

void QCPSyncer::rebuildConnections()
{
  for (const QPointer<QCPAxis> &axis : mConnectedAxes)
  {
    if (axis)
      disconnectAxis(axis);
  }
  mConnectedAxes.clear();
  const QList<QCPAxis*> axes = effectiveSyncedAxes();
  for (QCPAxis *axis : axes)
    connectAxis(axis);
}

void QCPSyncer::connectAxis(QCPAxis *axis)
{
  if (!axis || mConnectedAxes.contains(axis))
    return;
  connect(axis, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged),
          this, &QCPSyncer::onAxisRangeChanged);
  mConnectedAxes.append(axis);
}

void QCPSyncer::disconnectAxis(QCPAxis *axis)
{
  if (!axis)
    return;
  disconnect(axis, nullptr, this, nullptr);
}

void QCPSyncer::updateDragCursor(QCustomPlot *plot, const QPoint &pos)
{
  if (!plot)
    return;
  if (rulerAt(plot, pos) >= 0)
    plot->setCursor(Qt::SplitHCursor);
  else
    plot->unsetCursor();
}

QCPAxis *QCPSyncer::axisAt(QCustomPlot *plot, const QPoint &pos) const
{
  if (!plot)
    return nullptr;
  QCPAxis *fallback = nullptr;
  const QList<QCPAxis*> axes = effectiveSyncedAxes();
  for (QCPAxis *axis : axes)
  {
    if (!axis || axis->parentPlot() != plot)
      continue;
    QCPAxisRect *rect = axis->axisRect();
    if (rect && rect->rect().contains(pos))
      return axis;
    if (!fallback && rect)
      fallback = axis;
  }
  // Ruler lines span the whole widget, so they must stay grabbable in the margins and
  // in the gaps between axis rects as well. All synced rects share the same horizontal
  // scale, so any of them maps the x position correctly.
  return fallback;
}

void QCPSyncer::setRulerKeyInternal(int id, double key)
{
  QCPRuler *ruler = mRulers.value(id);
  if (!ruler)
    return;
  if (mHasKeyLimit)
    key = clampRange(QCPRange(key, key)).lower;
  if (qFuzzyCompare(ruler->key(), key))
    return;
  ruler->setKey(key);
  ruler->refresh();
  queueReplotAll();
  emit rulerKeyChanged(id, key);
}

void QCPSyncer::refreshAll()
{
  for (QCPRuler *ruler : mRulers)
    ruler->refresh();
}

void QCPSyncer::queueReplotAll()
{
  for (QCustomPlot *plot : mPlots)
  {
    if (plot)
      plot->replot(QCustomPlot::rpQueuedReplot);
  }
}
