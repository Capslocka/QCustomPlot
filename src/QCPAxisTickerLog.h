/* QCustomPlot 2.1.1 split source
   Copyright (C) 2011-2022 Emanuel Eichhammer
   Licensed under the GNU General Public License v3, see GPL.txt.
   Automatically split from the amalgamated qcustomplot.h/.cpp; keep code in sync. */

#ifndef QCPAXISTICKERLOG_H
#define QCPAXISTICKERLOG_H

#include "QCPGlobal.h"
#include "QCPRange.h"
#include "QCPAxisTicker.h"

class QCP_LIB_DECL QCPAxisTickerLog : public QCPAxisTicker
{
public:
  QCPAxisTickerLog();
  
  // getters:
  double logBase() const { return mLogBase; }
  int subTickCount() const { return mSubTickCount; }
  
  // setters:
  void setLogBase(double base);
  void setSubTickCount(int subTicks);
  
protected:
  // property members:
  double mLogBase;
  int mSubTickCount;
  
  // non-property members:
  double mLogBaseLnInv;
  
  // reimplemented virtual methods:
  virtual int getSubTickCount(double tickStep) Q_DECL_OVERRIDE;
  virtual QVector<double> createTickVector(double tickStep, const QCPRange &range) Q_DECL_OVERRIDE;
};


#endif // QCPAXISTICKERLOG_H
