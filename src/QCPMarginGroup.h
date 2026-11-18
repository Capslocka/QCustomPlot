/* QCustomPlot 2.1.1 split source
   Copyright (C) 2011-2022 Emanuel Eichhammer
   Licensed under the GNU General Public License v3, see GPL.txt.
   Automatically split from the amalgamated qcustomplot.h/.cpp; keep code in sync. */

#ifndef QCPMARGINGROUP_H
#define QCPMARGINGROUP_H

#include "QCPGlobal.h"

class QCP_LIB_DECL QCPMarginGroup : public QObject
{
  Q_OBJECT
public:
  explicit QCPMarginGroup(QCustomPlot *parentPlot);
  virtual ~QCPMarginGroup();
  
  // non-virtual methods:
  QList<QCPLayoutElement*> elements(QCP::MarginSide side) const { return mChildren.value(side); }
  bool isEmpty() const;
  void clear();
  
protected:
  // non-property members:
  QCustomPlot *mParentPlot;
  QHash<QCP::MarginSide, QList<QCPLayoutElement*> > mChildren;
  
  // introduced virtual methods:
  virtual int commonMargin(QCP::MarginSide side) const;
  
  // non-virtual methods:
  void addChild(QCP::MarginSide side, QCPLayoutElement *element);
  void removeChild(QCP::MarginSide side, QCPLayoutElement *element);
  
private:
  Q_DISABLE_COPY(QCPMarginGroup)
  
  friend class QCPLayoutElement;
};


#endif // QCPMARGINGROUP_H
