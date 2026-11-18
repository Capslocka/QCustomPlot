/* QCustomPlot 2.1.1 split source
   Copyright (C) 2011-2022 Emanuel Eichhammer
   Licensed under the GNU General Public License v3, see GPL.txt.
   Automatically split from the amalgamated qcustomplot.h/.cpp; keep code in sync. */

#ifndef QCPGRAPHDATA_H
#define QCPGRAPHDATA_H

#include "QCPGlobal.h"
#include "QCPRange.h"
#include "QCPDataContainer.h"

class QCP_LIB_DECL QCPGraphData
{
public:
  QCPGraphData();
  QCPGraphData(double key, double value);
  
  inline double sortKey() const { return key; }
  inline static QCPGraphData fromSortKey(double sortKey) { return QCPGraphData(sortKey, 0); }
  inline static bool sortKeyIsMainKey() { return true; }
  
  inline double mainKey() const { return key; }
  inline double mainValue() const { return value; }
  
  inline QCPRange valueRange() const { return QCPRange(value, value); }
  
  double key, value;
};
Q_DECLARE_TYPEINFO(QCPGraphData, Q_PRIMITIVE_TYPE);


/*! \typedef QCPGraphDataContainer
  
  Container for storing \ref QCPGraphData points. The data is stored sorted by \a key.
  
  This template instantiation is the container in which QCPGraph holds its data. For details about
  the generic container, see the documentation of the class template \ref QCPDataContainer.
  
  \see QCPGraphData, QCPGraph::setData
*/
typedef QCPDataContainer<QCPGraphData> QCPGraphDataContainer;


#endif // QCPGRAPHDATA_H
