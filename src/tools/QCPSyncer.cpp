/* QCustomPlot 2.1.1 split source
   Copyright (C) 2011-2022 Emanuel Eichhammer
   Licensed under the GNU General Public License v3, see GPL.txt.
   Automatically split from the amalgamated qcustomplot.h/.cpp; keep code in sync. */

#include "QCPSyncer_p.h"
#include "QCPAxisTickerDateTime.h"
#include "QCPLayer.h"

#include <QDateTime>
#include <QEvent>
#include <QMouseEvent>
#include <QtGlobal>

#include <algorithm>

namespace
{
const int RULER_GRAB_RADIUS = 6;
const int READOUT_LABEL_OFFSET = 8;

bool rangesEqual(const QCPRange &a, const QCPRange &b)
{
  return qFuzzyCompare(a.lower, b.lower) && qFuzzyCompare(a.upper, b.upper);
}

QString formatNumber(double value)
{
  return QString::number(value, 'g', 10);
}
} // namespace

// ---------------------------------------------------------------------------
// QCPRuler
// ---------------------------------------------------------------------------

QCPRuler::QCPRuler() :
  mKey(0),
  mVisible(true),
  mPen(QPen(Qt::red, 0, Qt::DashLine)),
  mXLabelItem(nullptr)
{
}

QCPRuler::~QCPRuler()
{
  QList<QCustomPlot*> plots = mItems.keys();
  for (QCustomPlot *plot : plots)
  {
    if (plot)
      clearPlotItems(plot);
  }
  discardLabelItem();
}

void QCPRuler::setKey(double key)
{
  mKey = key;
}

void QCPRuler::setVisible(bool visible)
{
  mVisible = visible;
}

void QCPRuler::setPen(const QPen &pen)
{
  mPen = pen;
}

void QCPRuler::setReadoutEnabled(QCPGraph *graph, bool enabled)
{
  if (!graph)
    return;
  if (enabled)
    mDisabledGraphs.remove(graph);
  else
    mDisabledGraphs.insert(graph, true);
}

bool QCPRuler::readoutEnabled(QCPGraph *graph) const
{
  return graph && !mDisabledGraphs.contains(graph);
}

void QCPRuler::setReadoutFormatter(std::function<QString(double)> formatter)
{
  mReadoutFormatter = formatter;
}

void QCPRuler::setXLabelFormatter(std::function<QString(double)> formatter)
{
  mXLabelFormatter = formatter;
}

void QCPRuler::setLabelPlot(QCustomPlot *plot)
{
  if (mLabelPlot == plot)
    return;
  discardLabelItem();
  mLabelPlot = plot;
  mLabelAxisRect = nullptr;
  ensureLabelItem();
}

void QCPRuler::setLabelAxisRect(QCPAxisRect *rect)
{
  mLabelAxisRect = rect;
  ensureLabelItem();
}

void QCPRuler::attachPlot(QCustomPlot *plot)
{
  if (!plot || mItems.contains(plot))
    return;
  mItems.insert(plot, PlotItems());
  createPlotItems(plot);
  if (!mLabelPlot)
    setLabelPlot(plot);
}

void QCPRuler::detachPlot(QCustomPlot *plot)
{
  if (!plot || !mItems.contains(plot))
    return;
  clearPlotItems(plot);
  mItems.remove(plot);
  if (mLabelPlot == plot)
  {
    mXLabelItem = nullptr;
    mLabelPlot = nullptr;
    mLabelAxisRect = nullptr;
  }
}

void QCPRuler::forgetPlot(QCustomPlot *plot)
{
  // The plot is already being destroyed, so its items must not be touched any more.
  mItems.remove(plot);
  if (mLabelPlot == plot)
  {
    mXLabelItem = nullptr;
    mLabelPlot = nullptr;
    mLabelAxisRect = nullptr;
  }
}

void QCPRuler::rescanPlot(QCustomPlot *plot)
{
  if (!plot || !mItems.contains(plot))
    return;
  clearReadoutItems(plot);
  PlotItems &items = mItems[plot];
  for (int i = 0; i < plot->graphCount(); ++i)
  {
    QCPGraph *graph = plot->graph(i);
    QCPItemTracer *tracer = new QCPItemTracer(plot);
    tracer->setLayer("overlay");
    tracer->setClipToAxisRect(false);
    tracer->setGraph(graph);
    tracer->setGraphKey(mKey);
    tracer->setInterpolating(true);
    tracer->setStyle(QCPItemTracer::tsCircle);
    tracer->setSize(8);

    QCPItemText *label = new QCPItemText(plot);
    label->setLayer("overlay");
    label->setClipToAxisRect(false);
    label->setPositionAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    label->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    label->setPadding(QMargins(3, 0, 3, 0));
    label->position->setParentAnchor(tracer->position);
    label->position->setCoords(READOUT_LABEL_OFFSET, 0);

    items.tracers.insert(graph, tracer);
    items.labels.insert(graph, label);
  }
  updatePlotVisuals(plot);
}

void QCPRuler::rescanAll()
{
  const QList<QCustomPlot*> plots = mItems.keys();
  for (QCustomPlot *plot : plots)
    rescanPlot(plot);
}

void QCPRuler::refreshPlot(QCustomPlot *plot)
{
  if (plot && mItems.contains(plot))
    updatePlotVisuals(plot);
}

void QCPRuler::refresh()
{
  const QList<QCustomPlot*> plots = mItems.keys();
  for (QCustomPlot *plot : plots)
    updatePlotVisuals(plot);

  if (mXLabelItem)
  {
    QCPAxisRect *rect = mLabelAxisRect;
    if (!rect && mLabelPlot)
      rect = mLabelPlot->axisRect();
    QCPAxis *axis = rect ? rect->axis(QCPAxis::atBottom) : nullptr;
    if (rect && axis)
    {
      const double x = axis->coordToPixel(mKey);
      const bool inside = mVisible && x >= rect->left() && x <= rect->right();
      mXLabelItem->setVisible(inside);
      if (inside)
      {
        mXLabelItem->setText(xLabelText(mKey, axis));
        mXLabelItem->position->setCoords(x, rect->bottom() + axis->offset() + 3);
        mXLabelItem->setPen(QPen(mPen.color()));
      }
    } else if (mXLabelItem)
    {
      mXLabelItem->setVisible(false);
    }
  }
}

double QCPRuler::interpolate(QCPGraph *graph, bool *ok) const
{
  if (ok)
    *ok = false;
  if (!graph || graph->data().isNull() || graph->data()->isEmpty())
    return 0;

  const QCPGraphDataContainer &container = *graph->data();
  QCPGraphDataContainer::const_iterator begin = container.constBegin();
  QCPGraphDataContainer::const_iterator last = container.constEnd() - 1;
  if (mKey < begin->key || mKey > last->key)
    return 0;

  QCPGraphDataContainer::const_iterator it = container.findBegin(mKey);
  double value = 0;
  if (it == container.constEnd())
  {
    value = last->value;
  } else if (it == begin || qFuzzyCompare(it->key, mKey))
  {
    value = it->value;
  } else
  {
    QCPGraphDataContainer::const_iterator prev = it - 1;
    const double span = it->key - prev->key;
    const double t = span > 0 ? (mKey - prev->key) / span : 0;
    value = prev->value + t * (it->value - prev->value);
  }
  if (ok)
    *ok = true;
  return value;
}

void QCPRuler::createPlotItems(QCustomPlot *plot)
{
  PlotItems &items = mItems[plot];
  for (int i = 0; i < plot->axisRectCount(); ++i)
  {
    QCPAxisRect *rect = plot->axisRect(i);
    QCPAxis *axis = rect->axis(QCPAxis::atBottom);
    if (!axis)
      continue;
    QCPItemStraightLine *line = new QCPItemStraightLine(plot);
    line->setLayer("overlay");
    line->setClipToAxisRect(true);
    line->setClipAxisRect(rect);
    line->setPen(mPen);
    line->point1->setType(QCPItemPosition::ptAxisRectRatio);
    line->point1->setAxisRect(rect);
    line->point2->setType(QCPItemPosition::ptAxisRectRatio);
    line->point2->setAxisRect(rect);
    line->point1->setCoords(0, 0);
    line->point2->setCoords(0, 1);
    items.lines.append(line);
    items.lineRects.append(rect);
  }
  rescanPlot(plot);
}

void QCPRuler::clearReadoutItems(QCustomPlot *plot)
{
  PlotItems &items = mItems[plot];
  for (QCPItemText *label : items.labels)
  {
    if (label)
    {
      label->position->setParentAnchor(nullptr);
      plot->removeItem(label); // deletes the item
    }
  }
  for (QCPItemTracer *tracer : items.tracers)
  {
    if (tracer)
    {
      plot->removeItem(tracer); // deletes the item
    }
  }
  items.tracers.clear();
  items.labels.clear();
}

void QCPRuler::clearPlotItems(QCustomPlot *plot)
{
  PlotItems &items = mItems[plot];
  clearReadoutItems(plot);
  for (QCPItemStraightLine *line : items.lines)
  {
    if (line)
    {
      plot->removeItem(line); // deletes the item
    }
  }
  items.lines.clear();
  items.lineRects.clear();
}

void QCPRuler::ensureLabelItem()
{
  if (mXLabelItem || !mLabelPlot)
    return;
  mXLabelItem = new QCPItemText(mLabelPlot);
  mXLabelItem->setLayer("overlay");
  mXLabelItem->setClipToAxisRect(false);
  mXLabelItem->setPositionAlignment(Qt::AlignHCenter | Qt::AlignTop);
  mXLabelItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignTop);
  mXLabelItem->setPadding(QMargins(3, 1, 3, 1));
  mXLabelItem->setBrush(QBrush(Qt::white));
  mXLabelItem->position->setType(QCPItemPosition::ptAbsolute);
  mXLabelItem->setVisible(false);
}

void QCPRuler::discardLabelItem()
{
  if (mXLabelItem)
  {
    QCustomPlot *plot = mXLabelItem->parentPlot();
    if (plot)
      plot->removeItem(mXLabelItem); // deletes the item
    mXLabelItem = nullptr;
  }
}

void QCPRuler::updatePlotVisuals(QCustomPlot *plot)
{
  if (!mItems.contains(plot))
    return;
  PlotItems &items = mItems[plot];

  for (int i = 0; i < items.lines.size(); ++i)
  {
    QCPItemStraightLine *line = items.lines.at(i);
    QCPAxisRect *rect = items.lineRects.at(i);
    QCPAxis *axis = rect ? rect->axis(QCPAxis::atBottom) : nullptr;
    if (!line || !rect || !axis)
      continue;
    const double ratio = (axis->coordToPixel(mKey) - rect->left()) / qMax(1, rect->width());
    line->point1->setCoords(ratio, 0);
    line->point2->setCoords(ratio, 1);
    line->setPen(mPen);
    line->setVisible(mVisible);
  }

  const QList<QCPGraph*> graphs = items.tracers.keys();
  for (QCPGraph *graph : graphs)
  {
    QCPItemTracer *tracer = items.tracers.value(graph);
    QCPItemText *label = items.labels.value(graph);
    if (!tracer || !label)
      continue;

    bool inRange = false;
    interpolate(graph, &inRange);
    const bool show = mVisible && inRange && readoutEnabled(graph);

    tracer->setGraphKey(mKey);
    tracer->setVisible(show);
    tracer->setPen(QPen(graph->pen().color()));

    label->setVisible(show);
    if (show)
    {
      label->setText(readoutText(graph, &inRange));
      label->setPen(QPen(graph->pen().color()));
      label->setBrush(QBrush(QColor(255, 255, 255, 220)));
      label->position->setCoords(READOUT_LABEL_OFFSET, 0);
    }
  }
}

QString QCPRuler::readoutText(QCPGraph *graph, bool *visible) const
{
  if (!graph || !visible || !*visible)
    return QString();
  QString text;
  if (mReadoutFormatter)
  {
    text = mReadoutFormatter(interpolate(graph, visible));
  } else
  {
    text = formatNumber(interpolate(graph, visible));
    const QString name = graph->name();
    if (!name.isEmpty())
      text = name + ": " + text;
  }
  return text;
}

QString QCPRuler::xLabelText(double key, QCPAxis *axis) const
{
  if (mXLabelFormatter)
    return mXLabelFormatter(key);
  if (axis)
  {
    QCPAxisTickerDateTime *dateTimeTicker = dynamic_cast<QCPAxisTickerDateTime*>(axis->ticker().data());
    if (dateTimeTicker)
      return QCPAxisTickerDateTime::keyToDateTime(key).toString(dateTimeTicker->dateTimeFormat());
  }
  return QStringLiteral("x = ") + formatNumber(key);
}

// ---------------------------------------------------------------------------
// QCPSyncer
// ---------------------------------------------------------------------------

QCPSyncer::QCPSyncer(QObject *parent) :
  QObject(parent),
  mSyncing(false),
  mRuler(new QCPRuler),
  mKeyInitialized(false),
  mDragging(false)
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
      mRuler->detachPlot(plot);
    }
  }
  delete mRuler;
}

void QCPSyncer::addPlot(QCustomPlot *plot)
{
  if (!plot || mPlots.contains(plot))
    return;
  mPlots.append(plot);
  plot->installEventFilter(this);
  plot->setMouseTracking(true);
  connect(plot, &QCustomPlot::beforeReplot, this, [this, plot]() {
    mRuler->refreshPlot(plot);
  });
  connect(plot, &QObject::destroyed, this, [this, plot]() {
    if (mPlots.removeAll(plot) > 0)
    {
      mConnectedAxes.clear();
      mExplicitSyncedAxes.clear();
      mRuler->forgetPlot(plot);
      if (mLabelPlot == plot)
        mLabelPlot = nullptr;
      if (mDragPlot == plot)
        mDragPlot = nullptr;
      rebuildConnections();
    }
  });

  mRuler->attachPlot(plot);
  if (!mLabelPlot)
    mLabelPlot = plot;
  mRuler->setLabelPlot(mLabelPlot);

  const QList<QCPAxis*> axes = effectiveSyncedAxes();
  QCPRange alignRange;
  bool hasAlignRange = false;
  for (QCPAxis *axis : axes)
  {
    if (axis->parentPlot() != plot)
    {
      alignRange = axis->range();
      hasAlignRange = true;
      break;
    }
  }
  if (hasAlignRange)
  {
    mSyncing = true;
    for (QCPAxis *axis : axes)
    {
      if (axis->parentPlot() == plot)
        axis->setRange(alignRange);
    }
    mSyncing = false;
  }

  if (!mKeyInitialized && !axes.isEmpty())
  {
    const QCPRange range = axes.first()->range();
    mRuler->setKey((range.lower + range.upper) * 0.5);
    mKeyInitialized = true;
  }

  rebuildConnections();
  mRuler->refresh();
  queueReplotAll();
}

void QCPSyncer::removePlot(QCustomPlot *plot)
{
  if (!plot || !mPlots.contains(plot))
    return;
  plot->removeEventFilter(this);
  mRuler->detachPlot(plot);
  mPlots.removeAll(plot);
  if (mLabelPlot == plot)
  {
    mLabelPlot = mPlots.isEmpty() ? nullptr : mPlots.first();
    mRuler->setLabelPlot(mLabelPlot);
  }
  if (mDragPlot == plot)
  {
    mDragging = false;
    mDragPlot = nullptr;
    mDragAxis = nullptr;
  }
  rebuildConnections();
  queueReplotAll();
}

QList<QCustomPlot*> QCPSyncer::plots() const
{
  return mPlots;
}

void QCPSyncer::setSyncedAxes(const QList<QCPAxis*> &axes)
{
  mExplicitSyncedAxes.clear();
  for (QCPAxis *axis : axes)
  {
    if (axis && mPlots.contains(axis->parentPlot()) && !mExplicitSyncedAxes.contains(axis))
      mExplicitSyncedAxes.append(axis);
  }
  rebuildConnections();
}

QList<QCPAxis*> QCPSyncer::syncedAxes() const
{
  return effectiveSyncedAxes();
}

void QCPSyncer::setRulerKey(double key)
{
  mKeyInitialized = true;
  setRulerKeyInternal(key);
}

double QCPSyncer::rulerKey() const
{
  return mRuler->key();
}

void QCPSyncer::setRulerVisible(bool visible)
{
  if (mRuler->visible() == visible)
    return;
  mRuler->setVisible(visible);
  mRuler->refresh();
  queueReplotAll();
}

bool QCPSyncer::rulerVisible() const
{
  return mRuler->visible();
}

void QCPSyncer::setRulerPen(const QPen &pen)
{
  mRuler->setPen(pen);
  mRuler->refresh();
  queueReplotAll();
}

QPen QCPSyncer::rulerPen() const
{
  return mRuler->pen();
}

void QCPSyncer::setReadoutEnabled(QCPGraph *graph, bool enabled)
{
  mRuler->setReadoutEnabled(graph, enabled);
  mRuler->refresh();
  queueReplotAll();
}

bool QCPSyncer::readoutEnabled(QCPGraph *graph) const
{
  return mRuler->readoutEnabled(graph);
}

void QCPSyncer::rescanGraphs()
{
  mRuler->rescanAll();
  mRuler->refresh();
  queueReplotAll();
}

void QCPSyncer::setLabelPlot(QCustomPlot *plot)
{
  if (!plot || !mPlots.contains(plot))
    return;
  mLabelPlot = plot;
  mRuler->setLabelPlot(plot);
  mRuler->refresh();
  queueReplotAll();
}

QCustomPlot *QCPSyncer::labelPlot() const
{
  return mLabelPlot;
}

void QCPSyncer::setLabelAxisRect(QCPAxisRect *rect)
{
  mLabelAxisRect = rect;
  mRuler->setLabelAxisRect(rect);
  mRuler->refresh();
  queueReplotAll();
}

QCPAxisRect *QCPSyncer::labelAxisRect() const
{
  if (mLabelAxisRect)
    return mLabelAxisRect;
  return mLabelPlot ? mLabelPlot->axisRect() : nullptr;
}

void QCPSyncer::setReadoutFormatter(std::function<QString(double)> formatter)
{
  mReadoutFormatter = formatter;
  mRuler->setReadoutFormatter(formatter);
  mRuler->refresh();
  queueReplotAll();
}

void QCPSyncer::setXLabelFormatter(std::function<QString(double)> formatter)
{
  mXLabelFormatter = formatter;
  mRuler->setXLabelFormatter(formatter);
  mRuler->refresh();
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
      QCPAxis *axis = axisAt(plot, pos);
      if (!axis)
        break;
      if (qAbs(axis->coordToPixel(mRuler->key()) - pos.x()) > RULER_GRAB_RADIUS)
        break;
      mDragging = true;
      mDragPlot = plot;
      mDragAxis = axis;
      plot->setCursor(Qt::SplitHCursor);
      double key = axis->pixelToCoord(pos.x());
      key = qBound(qMin(axis->range().lower, axis->range().upper),
                   key, qMax(axis->range().lower, axis->range().upper));
      setRulerKeyInternal(key);
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
        setRulerKeyInternal(key);
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

void QCPSyncer::applyRange(const QCPRange &range, QCPAxis *source)
{
  mSyncing = true;
  const QList<QCPAxis*> axes = effectiveSyncedAxes();
  for (QCPAxis *axis : axes)
  {
    if (axis && axis != source && !rangesEqual(axis->range(), range))
      axis->setRange(range);
  }
  mSyncing = false;
  emit syncedRangeChanged(range);
  mRuler->refresh();
  queueReplotAll();
}

QList<QCPAxis*> QCPSyncer::effectiveSyncedAxes() const
{
  QList<QCPAxis*> result;
  if (!mExplicitSyncedAxes.isEmpty())
  {
    for (QCPAxis *axis : mExplicitSyncedAxes)
    {
      if (axis && mPlots.contains(axis->parentPlot()))
        result.append(axis);
    }
    return result;
  }
  for (QCustomPlot *plot : mPlots)
  {
    if (!plot)
      continue;
    for (int i = 0; i < plot->axisRectCount(); ++i)
    {
      QCPAxis *axis = plot->axisRect(i)->axis(QCPAxis::atBottom);
      if (axis && !result.contains(axis))
        result.append(axis);
    }
  }
  return result;
}

void QCPSyncer::rebuildConnections()
{
  for (QCPAxis *axis : mConnectedAxes)
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
  QCPAxis *axis = axisAt(plot, pos);
  if (axis && qAbs(axis->coordToPixel(mRuler->key()) - pos.x()) <= RULER_GRAB_RADIUS)
    plot->setCursor(Qt::SplitHCursor);
  else
    plot->unsetCursor();
}

QCPAxis *QCPSyncer::axisAt(QCustomPlot *plot, const QPoint &pos) const
{
  if (!plot)
    return nullptr;
  const QList<QCPAxis*> axes = effectiveSyncedAxes();
  for (QCPAxis *axis : axes)
  {
    if (!axis || axis->parentPlot() != plot)
      continue;
    QCPAxisRect *rect = axis->axisRect();
    if (rect && rect->rect().contains(pos))
      return axis;
  }
  return nullptr;
}

void QCPSyncer::setRulerKeyInternal(double key)
{
  if (qFuzzyCompare(mRuler->key(), key) && mKeyInitialized)
    return;
  mRuler->setKey(key);
  mRuler->refresh();
  queueReplotAll();
  emit rulerKeyChanged(key);
}

void QCPSyncer::queueReplotAll()
{
  for (QCustomPlot *plot : mPlots)
  {
    if (plot)
      plot->replot(QCustomPlot::rpQueuedReplot);
  }
}
