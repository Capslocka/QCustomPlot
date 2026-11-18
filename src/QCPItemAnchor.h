/* QCustomPlot 2.1.1 split source
   Copyright (C) 2011-2022 Emanuel Eichhammer
   Licensed under the GNU General Public License v3, see GPL.txt.
   Automatically split from the amalgamated qcustomplot.h/.cpp; keep code in sync. */

#ifndef QCPITEMANCHOR_H
#define QCPITEMANCHOR_H

#include "QCPGlobal.h"

class QCP_LIB_DECL QCPItemAnchor
{
  Q_GADGET
public:
  QCPItemAnchor(QCustomPlot *parentPlot, QCPAbstractItem *parentItem, const QString &name, int anchorId=-1);
  virtual ~QCPItemAnchor();
  
  // getters:
  QString name() const { return mName; }
  virtual QPointF pixelPosition() const;
  
protected:
  // property members:
  QString mName;
  
  // non-property members:
  QCustomPlot *mParentPlot;
  QCPAbstractItem *mParentItem;
  int mAnchorId;
  QSet<QCPItemPosition*> mChildrenX, mChildrenY;
  
  // introduced virtual methods:
  virtual QCPItemPosition *toQCPItemPosition() { return nullptr; }
  
  // non-virtual methods:
  void addChildX(QCPItemPosition* pos); // called from pos when this anchor is set as parent
  void removeChildX(QCPItemPosition *pos); // called from pos when its parent anchor is reset or pos deleted
  void addChildY(QCPItemPosition* pos); // called from pos when this anchor is set as parent
  void removeChildY(QCPItemPosition *pos); // called from pos when its parent anchor is reset or pos deleted
  
private:
  Q_DISABLE_COPY(QCPItemAnchor)
  
  friend class QCPItemPosition;
};


#endif // QCPITEMANCHOR_H
