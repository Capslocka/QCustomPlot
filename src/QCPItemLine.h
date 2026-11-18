/* QCustomPlot 2.1.1 split source
   Copyright (C) 2011-2022 Emanuel Eichhammer
   Licensed under the GNU General Public License v3, see GPL.txt.
   Automatically split from the amalgamated qcustomplot.h/.cpp; keep code in sync. */

#ifndef QCPITEMLINE_H
#define QCPITEMLINE_H

#include "QCPGlobal.h"
#include "QCPVector2D.h"
#include "QCPPainter.h"
#include "QCPLineEnding.h"
#include "QCPItemPosition.h"
#include "QCPAbstractItem.h"
#include "QCustomPlot.h"

class QCP_LIB_DECL QCPItemLine : public QCPAbstractItem
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(QPen pen READ pen WRITE setPen)
  Q_PROPERTY(QPen selectedPen READ selectedPen WRITE setSelectedPen)
  Q_PROPERTY(QCPLineEnding head READ head WRITE setHead)
  Q_PROPERTY(QCPLineEnding tail READ tail WRITE setTail)
  /// \endcond
public:
  explicit QCPItemLine(QCustomPlot *parentPlot);
  virtual ~QCPItemLine() Q_DECL_OVERRIDE;
  
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
  QCPItemPosition * const end;
  
protected:
  // property members:
  QPen mPen, mSelectedPen;
  QCPLineEnding mHead, mTail;
  
  // reimplemented virtual methods:
  virtual void draw(QCPPainter *painter) Q_DECL_OVERRIDE;
  
  // non-virtual methods:
  QLineF getRectClippedLine(const QCPVector2D &start, const QCPVector2D &end, const QRect &rect) const;
  QPen mainPen() const;
};


#endif // QCPITEMLINE_H
