/* QCustomPlot 2.1.1 split source
   Copyright (C) 2011-2022 Emanuel Eichhammer
   Licensed under the GNU General Public License v3, see GPL.txt.
   Automatically split from the amalgamated qcustomplot.h/.cpp; keep code in sync. */

#ifndef QCPERRORBARSDATA_H
#define QCPERRORBARSDATA_H

#include "QCPGlobal.h"

class QCP_LIB_DECL QCPErrorBarsData
{
public:
  QCPErrorBarsData();
  explicit QCPErrorBarsData(double error);
  QCPErrorBarsData(double errorMinus, double errorPlus);
  
  double errorMinus, errorPlus;
};
Q_DECLARE_TYPEINFO(QCPErrorBarsData, Q_PRIMITIVE_TYPE);


/*! \typedef QCPErrorBarsDataContainer

  Container for storing \ref QCPErrorBarsData points. It is a typedef for <tt>QVector<\ref
  QCPErrorBarsData></tt>.

  This is the container in which \ref QCPErrorBars holds its data. Unlike most other data
  containers for plottables, it is not based on \ref QCPDataContainer. This is because the error
  bars plottable is special in that it doesn't store its own key and value coordinate per error
  bar. It adopts the key and value from the plottable to which the error bars shall be applied
  (\ref QCPErrorBars::setDataPlottable). So the stored \ref QCPErrorBarsData doesn't need a
  sortable key, but merely an index (as \c QVector provides), which maps one-to-one to the indices
  of the other plottable's data.

  \see QCPErrorBarsData, QCPErrorBars::setData
*/
typedef QVector<QCPErrorBarsData> QCPErrorBarsDataContainer;


#endif // QCPERRORBARSDATA_H
