/* QCustomPlot 2.1.1 split source
   Copyright (C) 2011-2022 Emanuel Eichhammer
   Licensed under the GNU General Public License v3, see GPL.txt.
   Automatically split from the amalgamated qcustomplot.h/.cpp; keep code in sync. */

#ifndef QCPITEMELLIPSE_H
#define QCPITEMELLIPSE_H

#include "QCPGlobal.h"
#include "QCPPainter.h"
#include "QCPItemAnchor.h"
#include "QCPItemPosition.h"
#include "QCPAbstractItem.h"
#include "QCustomPlot.h"

class QCP_LIB_DECL QCPItemEllipse : public QCPAbstractItem
{
  Q_OBJECT
  /// \cond INCLUDE_QPROPERTIES
  Q_PROPERTY(QPen pen READ pen WRITE setPen)
  Q_PROPERTY(QPen selectedPen READ selectedPen WRITE setSelectedPen)
  Q_PROPERTY(QBrush brush READ brush WRITE setBrush)
  Q_PROPERTY(QBrush selectedBrush READ selectedBrush WRITE setSelectedBrush)
  /// \endcond
public:
  explicit QCPItemEllipse(QCustomPlot *parentPlot);
  virtual ~QCPItemEllipse() Q_DECL_OVERRIDE;
  
  // getters:
  QPen pen() const { return mPen; }
  QPen selectedPen() const { return mSelectedPen; }
  QBrush brush() const { return mBrush; }
  QBrush selectedBrush() const { return mSelectedBrush; }
  
  // setters;
  void setPen(const QPen &pen);
  void setSelectedPen(const QPen &pen);
  void setBrush(const QBrush &brush);
  void setSelectedBrush(const QBrush &brush);
  
  // reimplemented virtual methods:
  virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=nullptr) const Q_DECL_OVERRIDE;
  
  QCPItemPosition * const topLeft;
  QCPItemPosition * const bottomRight;
  QCPItemAnchor * const topLeftRim;
  QCPItemAnchor * const top;
  QCPItemAnchor * const topRightRim;
  QCPItemAnchor * const right;
  QCPItemAnchor * const bottomRightRim;
  QCPItemAnchor * const bottom;
  QCPItemAnchor * const bottomLeftRim;
  QCPItemAnchor * const left;
  QCPItemAnchor * const center;
  
protected:
  enum AnchorIndex {aiTopLeftRim, aiTop, aiTopRightRim, aiRight, aiBottomRightRim, aiBottom, aiBottomLeftRim, aiLeft, aiCenter};
  
  // property members:
  QPen mPen, mSelectedPen;
  QBrush mBrush, mSelectedBrush;
  
  // reimplemented virtual methods:
  virtual void draw(QCPPainter *painter) Q_DECL_OVERRIDE;
  virtual QPointF anchorPixelPosition(int anchorId) const Q_DECL_OVERRIDE;
  
  // non-virtual methods:
  QPen mainPen() const;
  QBrush mainBrush() const;
};


#endif // QCPITEMELLIPSE_H
