/* QCustomPlot 2.1.1 split source
   Copyright (C) 2011-2022 Emanuel Eichhammer
   Licensed under the GNU General Public License v3, see GPL.txt.
   Automatically split from the amalgamated qcustomplot.h/.cpp; keep code in sync. */

#ifndef QCPITEMCURVE_H
#define QCPITEMCURVE_H

#include "QCPGlobal.h"
#include "QCPPainter.h"
#include "QCPLineEnding.h"
#include "QCPItemPosition.h"
#include "QCPAbstractItem.h"
#include "QCustomPlot.h"

class QCP_LIB_DECL QCPItemCurve : public QCPAbstractItem
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(QPen pen READ pen WRITE setPen)
  Q_PROPERTY(QPen selectedPen READ selectedPen WRITE setSelectedPen)
  Q_PROPERTY(QCPLineEnding head READ head WRITE setHead)
  Q_PROPERTY(QCPLineEnding tail READ tail WRITE setTail)
  /// \endcond
public:
  explicit QCPItemCurve(QCustomPlot *parentPlot);
  virtual ~QCPItemCurve() Q_DECL_OVERRIDE;
  
  // getters:
  QPen pen() const { return mPen; }
  QPen selectedPen() const { return mSelectedPen; }
  QCPLineEnding head() const { return mHead; }
  QCPLineEnding tail() const { return mTail; }
  
  // setters;
  void setPen(const QPen &pen);
  void setSelectedPen(const QPen &pen);
  void setHead(const QCPLineEnding &head);
  void setTail(const QCPLineEnding &tail);
  
  // reimplemented virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=nullptr) const Q_DECL_OVERRIDE;
  
  QCPItemPosition * const start;
  QCPItemPosition * const startDir;
  QCPItemPosition * const endDir;
  QCPItemPosition * const end;
  
protected:
  // property members:
  QPen mPen, mSelectedPen;
  QCPLineEnding mHead, mTail;
  
  // reimplemented virtual methods:
  virtual void draw(QCPPainter *painter) Q_DECL_OVERRIDE;
  
  // non-virtual methods:
  QPen mainPen() const;
};


#endif // QCPITEMCURVE_H
