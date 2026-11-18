/* QCustomPlot 2.1.1 split source
   Copyright (C) 2011-2022 Emanuel Eichhammer
   Licensed under the GNU General Public License v3, see GPL.txt.
   Automatically split from the amalgamated qcustomplot.h/.cpp; keep code in sync. */

#ifndef QCPBARSDATA_H
#define QCPBARSDATA_H

#include "QCPGlobal.h"
#include "QCPRange.h"
#include "QCPDataContainer.h"

class QCP_LIB_DECL QCPBarsData
{
public:
  QCPBarsData();
  QCPBarsData(double key, double value);
  
  inline double sortKey() const { return key; }
  inline static QCPBarsData fromSortKey(double sortKey) { return QCPBarsData(sortKey, 0); }
  inline static bool sortKeyIsMainKey() { return true; } 
  
  inline double mainKey() const { return key; }
  inline double mainValue() const { return value; }
  
  inline QCPRange valueRange() const { return QCPRange(value, value); } // note that bar base value isn't held in each QCPBarsData and thus can't/shouldn't be returned here
  
  double key, value;
};
Q_DECLARE_TYPEINFO(QCPBarsData, Q_PRIMITIVE_TYPE);


/*! \typedef QCPBarsDataContainer
  
  Container for storing \ref QCPBarsData points. The data is stored sorted by \a key.
  
  This template instantiation is the container in which QCPBars holds its data. For details about
  the generic container, see the documentation of the class template \ref QCPDataContainer.
  
  \see QCPBarsData, QCPBars::setData
*/
typedef QCPDataContainer<QCPBarsData> QCPBarsDataContainer;


#endif // QCPBARSDATA_H
