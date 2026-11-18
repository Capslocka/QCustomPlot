/* QCustomPlot 2.1.1 split source
   Copyright (C) 2011-2022 Emanuel Eichhammer
   Licensed under the GNU General Public License v3, see GPL.txt.
   Automatically split from the amalgamated qcustomplot.h/.cpp; keep code in sync. */

#ifndef QCPPLOTTABLEINTERFACE1D_H
#define QCPPLOTTABLEINTERFACE1D_H

#include "QCPGlobal.h"
#include "QCPRange.h"
#include "QCPDataSelection.h"

class QCPPlottableInterface1D
{
public:
  virtual ~QCPPlottableInterface1D() = default;
  // introduced pure virtual methods:
  virtual int dataCount() const = 0;
  virtual double dataMainKey(int index) const = 0;
  virtual double dataSortKey(int index) const = 0;
  virtual double dataMainValue(int index) const = 0;
  virtual QCPRange dataValueRange(int index) const = 0;
  virtual QPointF dataPixelPosition(int index) const = 0;
  virtual bool sortKeyIsMainKey() const = 0;
  virtual QCPDataSelection selectTestRect(const QRectF &rect, bool onlySelectable) const = 0;
  virtual int findBegin(double sortKey, bool expandedRange=true) const = 0;
  virtual int findEnd(double sortKey, bool expandedRange=true) const = 0;
};


#endif // QCPPLOTTABLEINTERFACE1D_H
