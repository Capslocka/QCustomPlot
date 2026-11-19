/* QCustomPlot 2.1.1 split source
   Copyright (C) 2011-2022 Emanuel Eichhammer
   Licensed under the GNU General Public License v3, see GPL.txt.
   Automatically split from the amalgamated qcustomplot.h/.cpp; keep code in sync. */

#include "QCPRuler.h"
#include "QCPAxisTickerDateTime.h"
#include <QFontMetrics>
#include <QtGlobal>

namespace
{
const int READOUT_LABEL_OFFSET = 8;

QString formatNumber(double value)
{
  return QString::number(value, 'g', 10);
}
} // namespace

QCPRuler::QCPRuler(int id) :
  mId(id),
  mKey(0),
  mVisible(true),
  mActive(false),
  mColor(Qt::red),
  mXLabelPlacement(xlpBelowAxis)
{
}

QCPRuler::~QCPRuler()
{
  const QList<QCustomPlot*> plots = mItems.keys();
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

void QCPRuler::setActive(bool active)
{
  mActive = active;
}

void QCPRuler::setColor(const QColor &color)
{
  mColor = color;
}

void QCPRuler::setReadoutEnabled(QCPGraph *graph, bool enabled)
{
  if (!graph)
    return;
  for (int i = mDisabledGraphs.size() - 1; i >= 0; --i)
  {
    if (mDisabledGraphs.at(i).isNull() || mDisabledGraphs.at(i).data() == graph)
      mDisabledGraphs.removeAt(i);
  }
  if (!enabled)
    mDisabledGraphs.append(graph);
}

bool QCPRuler::readoutEnabled(QCPGraph *graph)
{
  if (!graph)
    return false;
  for (int i = mDisabledGraphs.size() - 1; i >= 0; --i)
  {
    if (mDisabledGraphs.at(i).isNull())
      mDisabledGraphs.removeAt(i);
    else if (mDisabledGraphs.at(i).data() == graph)
      return false;
  }
  return true;
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

void QCPRuler::setXLabelPlacement(XLabelPlacement placement)
{
  mXLabelPlacement = placement;
}

void QCPRuler::attachPlot(QCustomPlot *plot)
{
  if (!plot || mItems.contains(plot))
    return;
  mItems.insert(plot, PlotItems());
  createPlotItems(plot);
}

void QCPRuler::detachPlot(QCustomPlot *plot)
{
  if (!plot || !mItems.contains(plot))
    return;
  clearPlotItems(plot);
  mItems.remove(plot);
  if (mLabelPlot == plot)
  {
    discardLabelItem();
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
    tracer->setSize(6);
    tracer->setPen(QPen(graph->pen().color()));
    tracer->setBrush(QBrush(graph->pen().color()));

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
  if (!plot || !mItems.contains(plot))
    return;
  updatePlotVisuals(plot);
  if (mLabelPlot == plot)
    updateXLabel();
}

void QCPRuler::refresh()
{
  const QList<QCustomPlot*> plots = mItems.keys();
  for (QCustomPlot *plot : plots)
    updatePlotVisuals(plot);
  updateXLabel();
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
  QCPItemStraightLine *line = new QCPItemStraightLine(plot);
  line->setLayer("overlay");
  line->setClipToAxisRect(false);
  line->setPen(linePen());
  line->point1->setType(QCPItemPosition::ptAbsolute);
  line->point2->setType(QCPItemPosition::ptAbsolute);
  line->point1->setCoords(0, 0);
  line->point2->setCoords(0, 1);
  line->setVisible(false);
  items.line = line;
  rescanPlot(plot);
}

void QCPRuler::clearReadoutItems(QCustomPlot *plot)
{
  PlotItems &items = mItems[plot];
  const QList<QPointer<QCPItemText> > labels = items.labels.values();
  for (const QPointer<QCPItemText> &label : labels)
  {
    if (label)
    {
      label->position->setParentAnchor(nullptr);
      plot->removeItem(label); // deletes the item
    }
  }
  const QList<QPointer<QCPItemTracer> > tracers = items.tracers.values();
  for (const QPointer<QCPItemTracer> &tracer : tracers)
  {
    if (tracer)
      plot->removeItem(tracer); // deletes the item
  }
  items.tracers.clear();
  items.labels.clear();
}

void QCPRuler::clearPlotItems(QCustomPlot *plot)
{
  PlotItems &items = mItems[plot];
  clearReadoutItems(plot);
  if (items.line)
    plot->removeItem(items.line); // deletes the item
  items.line = nullptr;
}

void QCPRuler::ensureLabelItem()
{
  if (mXLabelItem || !mLabelPlot)
    return;
  mXLabelItem = new QCPItemText(mLabelPlot);
  mXLabelItem->setLayer("overlay");
  mXLabelItem->setClipToAxisRect(false);
  mXLabelItem->setPadding(QMargins(3, 1, 3, 1));
  mXLabelItem->setBrush(QBrush(QColor(255, 255, 255, 220)));
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

QPen QCPRuler::linePen() const
{
  return QPen(mColor, mActive ? 2.5 : 1.5, Qt::DotLine);
}

void QCPRuler::updatePlotVisuals(QCustomPlot *plot)
{
  if (!plot || !mItems.contains(plot))
    return;
  PlotItems &items = mItems[plot];

  QCPAxisRect *rect = plot->axisRectCount() > 0 ? plot->axisRect(0) : nullptr;
  QCPAxis *axis = rect ? rect->axis(QCPAxis::atBottom) : nullptr;
  const bool hasAxis = axis && rect->width() > 0;
  const double x = hasAxis ? axis->coordToPixel(mKey) : 0.0;
  const bool inside = hasAxis && x >= rect->left() && x <= rect->right() + 1;

  const QPen pen = linePen();
  if (QCPItemStraightLine *line = items.line)
  {
    if (hasAxis)
    {
      line->point1->setCoords(x, 0);
      line->point2->setCoords(x, plot->height());
      line->setPen(pen);
    }
    line->setVisible(mVisible && inside);
  }

  for (int i = 0; i < plot->graphCount(); ++i)
  {
    QCPGraph *graph = plot->graph(i);
    QCPItemTracer *tracer = items.tracers.value(graph);
    QCPItemText *label = items.labels.value(graph);
    if (!tracer || !label)
      continue;

    bool inData = false;
    interpolate(graph, &inData);
    const bool show = mVisible && inData && inside && readoutEnabled(graph);

    tracer->setGraphKey(mKey);
    tracer->setVisible(show);
    tracer->setPen(QPen(graph->pen().color()));
    tracer->setBrush(QBrush(graph->pen().color()));

    label->setVisible(show);
    if (show)
    {
      label->setText(readoutText(graph, &inData));
      label->setPen(QPen(graph->pen().color()));
      label->setBrush(QBrush(QColor(255, 255, 255, 220)));

      const QFontMetrics metrics(label->font());
      const QMargins padding = label->padding();
      const int labelWidth = metrics.horizontalAdvance(label->text())
          + padding.left() + padding.right();
      const int viewWidth = plot->viewport().width();
      const int edge = 2;
      if (viewWidth > 0 && x + READOUT_LABEL_OFFSET + labelWidth > viewWidth - edge)
      {
        label->setPositionAlignment(Qt::AlignRight | Qt::AlignVCenter);
        label->position->setCoords(-READOUT_LABEL_OFFSET, 0);
      } else
      {
        label->setPositionAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        label->position->setCoords(READOUT_LABEL_OFFSET, 0);
      }
    }
  }
}

void QCPRuler::updateXLabel()
{
  ensureLabelItem();
  if (!mXLabelItem)
    return;

  QCPAxisRect *rect = mLabelAxisRect;
  if (!rect && mLabelPlot)
    rect = mLabelPlot->axisRect();
  QCPAxis *axis = rect ? rect->axis(QCPAxis::atBottom) : nullptr;
  if (!rect || !axis)
  {
    mXLabelItem->setVisible(false);
    return;
  }

  const double x = axis->coordToPixel(mKey);
  const bool inside = mVisible && x >= rect->left() && x <= rect->right() + 1;
  mXLabelItem->setVisible(inside);
  if (!inside)
    return;

  mXLabelItem->setText(xLabelText(mKey, axis));

  // The label is never clipped to the axis rect, so the widget viewport is the only
  // boundary that can cut it off.
  QCustomPlot *labelPlot = mXLabelItem->parentPlot();
  const int viewWidth = labelPlot ? labelPlot->viewport().width() : 0;
  const int viewHeight = labelPlot ? labelPlot->viewport().height() : 0;
  const QFontMetrics metrics(mXLabelItem->font());
  const QMargins padding = mXLabelItem->padding();
  const int labelWidth = metrics.horizontalAdvance(mXLabelItem->text())
      + padding.left() + padding.right();
  const int gap = 5;
  const int edge = 2;

  if (mXLabelPlacement == xlpRectInside)
  {
    // Bottom area of the label widget, to the right of the ruler line. Near the right
    // edge the label flips to the left side so it stays fully visible.
    const double y = viewHeight - 3;
    mXLabelItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    if (viewWidth > 0 && x + gap + labelWidth > viewWidth - edge)
    {
      mXLabelItem->setPositionAlignment(Qt::AlignRight | Qt::AlignBottom);
      mXLabelItem->position->setCoords(x - gap, y);
    } else
    {
      mXLabelItem->setPositionAlignment(Qt::AlignLeft | Qt::AlignBottom);
      mXLabelItem->position->setCoords(x + gap, y);
    }
  } else
  {
    const double y = rect->bottom() + axis->offset() + 3;
    mXLabelItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    if (viewWidth > 0 && x + labelWidth / 2.0 > viewWidth - edge)
    {
      mXLabelItem->setPositionAlignment(Qt::AlignRight | Qt::AlignTop);
      mXLabelItem->position->setCoords(x - gap, y);
    } else
    {
      mXLabelItem->setPositionAlignment(Qt::AlignHCenter | Qt::AlignTop);
      mXLabelItem->position->setCoords(x, y);
    }
  }
  mXLabelItem->setPen(QPen(mColor));
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
    if (QCPAxisTickerDateTime *ticker = dynamic_cast<QCPAxisTickerDateTime*>(axis->ticker().data()))
      return ticker->keyToDateTime(key).toString(ticker->dateTimeFormat());
  }
  return QStringLiteral("x = ") + formatNumber(key);
}
