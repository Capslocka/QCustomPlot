/* QCustomPlot 2.1.1 split source
   Copyright (C) 2011-2022 Emanuel Eichhammer
   Licensed under the GNU General Public License v3, see GPL.txt.
   Automatically split from the amalgamated qcustomplot.h/.cpp; keep code in sync. */

#ifndef QCPSTATISTICALBOXDATA_H
#define QCPSTATISTICALBOXDATA_H

#include "QCPGlobal.h"
#include "QCPRange.h"
#include "QCPDataContainer.h"

class QCP_LIB_DECL QCPStatisticalBoxData
{
public:
  QCPStatisticalBoxData();
  QCPStatisticalBoxData(double key, double minimum, double lowerQuartile, double median, double upperQuartile, double maximum, const QVector<double>& outliers=QVector<double>());
  
  inline double sortKey() const { return key; }
  inline static QCPStatisticalBoxData fromSortKey(double sortKey) { return QCPStatisticalBoxData(sortKey, 0, 0, 0, 0, 0); }
  inline static bool sortKeyIsMainKey() { return true; }
  
  inline double mainKey() const { return key; }
  inline double mainValue() const { return median; }
  
  inline QCPRange valueRange() const
  {
    QCPRange result(minimum, maximum);
    for (QVector<double>::const_iterator it = outliers.constBegin(); it != outliers.constEnd(); ++it)
      result.expand(*it);
    return result;
  }
  
  double key, minimum, lowerQuartile, median, upperQuartile, maximum;
  QVector<double> outliers;
};
Q_DECLARE_TYPEINFO(QCPStatisticalBoxData, Q_MOVABLE_TYPE);


/*! \typedef QCPStatisticalBoxDataContainer
  
  Container for storing \ref QCPStatisticalBoxData points. The data is stored sorted by \a key.
  
  This template instantiation is the container in which QCPStatisticalBox holds its data. For
  details about the generic container, see the documentation of the class template \ref
  QCPDataContainer.
  
  \see QCPStatisticalBoxData, QCPStatisticalBox::setData
*/
typedef QCPDataContainer<QCPStatisticalBoxData> QCPStatisticalBoxDataContainer;


#endif // QCPSTATISTICALBOXDATA_H
