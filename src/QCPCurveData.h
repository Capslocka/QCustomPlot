/* QCustomPlot 2.1.1 split source
   Copyright (C) 2011-2022 Emanuel Eichhammer
   Licensed under the GNU General Public License v3, see GPL.txt.
   Automatically split from the amalgamated qcustomplot.h/.cpp; keep code in sync. */

#ifndef QCPCURVEDATA_H
#define QCPCURVEDATA_H

#include "QCPGlobal.h"
#include "QCPRange.h"
#include "QCPDataContainer.h"

class QCP_LIB_DECL QCPCurveData
{
public:
  QCPCurveData();
  QCPCurveData(double t, double key, double value);
  
  inline double sortKey() const { return t; }
  inline static QCPCurveData fromSortKey(double sortKey) { return QCPCurveData(sortKey, 0, 0); }
  inline static bool sortKeyIsMainKey() { return false; }
  
  inline double mainKey() const { return key; }
  inline double mainValue() const { return value; }
  
  inline QCPRange valueRange() const { return QCPRange(value, value); }
  
  double t, key, value;
};
Q_DECLARE_TYPEINFO(QCPCurveData, Q_PRIMITIVE_TYPE);


/*! \typedef QCPCurveDataContainer
  
  Container for storing \ref QCPCurveData points. The data is stored sorted by \a t, so the \a
  sortKey() (returning \a t) is different from \a mainKey() (returning \a key).
  
  This template instantiation is the container in which QCPCurve holds its data. For details about
  the generic container, see the documentation of the class template \ref QCPDataContainer.
  
  \see QCPCurveData, QCPCurve::setData
*/
typedef QCPDataContainer<QCPCurveData> QCPCurveDataContainer;


#endif // QCPCURVEDATA_H
