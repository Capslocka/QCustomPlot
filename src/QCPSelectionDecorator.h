/* QCustomPlot 2.1.1 split source
   Copyright (C) 2011-2022 Emanuel Eichhammer
   Licensed under the GNU General Public License v3, see GPL.txt.
   Automatically split from the amalgamated qcustomplot.h/.cpp; keep code in sync. */

#ifndef QCPSELECTIONDECORATOR_H
#define QCPSELECTIONDECORATOR_H

#include "QCPGlobal.h"
#include "QCPPainter.h"
#include "QCPDataSelection.h"
#include "QCPScatterStyle.h"

class QCP_LIB_DECL QCPSelectionDecorator
{
  Q_GADGET
public:
  QCPSelectionDecorator();
  virtual ~QCPSelectionDecorator();
  
  // getters:
  QPen pen() const { return mPen; }
  QBrush brush() const { return mBrush; }
  QCPScatterStyle scatterStyle() const { return mScatterStyle; }
  QCPScatterStyle::ScatterProperties usedScatterProperties() const { return mUsedScatterProperties; }
  
  // setters:
  void setPen(const QPen &pen);
  void setBrush(const QBrush &brush);
  void setScatterStyle(const QCPScatterStyle &scatterStyle, QCPScatterStyle::ScatterProperties usedProperties=QCPScatterStyle::spPen);
  void setUsedScatterProperties(const QCPScatterStyle::ScatterProperties &properties);
  
  // non-virtual methods:
  void applyPen(QCPPainter *painter) const;
  void applyBrush(QCPPainter *painter) const;
  QCPScatterStyle getFinalScatterStyle(const QCPScatterStyle &unselectedStyle) const;
  
  // introduced virtual methods:
  virtual void copyFrom(const QCPSelectionDecorator *other);
  virtual void drawDecoration(QCPPainter *painter, QCPDataSelection selection);
  
protected:
  // property members:
  QPen mPen;
  QBrush mBrush;
  QCPScatterStyle mScatterStyle;
  QCPScatterStyle::ScatterProperties mUsedScatterProperties;
  // non-property members:
  QCPAbstractPlottable *mPlottable;
  
  // introduced virtual methods:
  virtual bool registerWithPlottable(QCPAbstractPlottable *plottable);
  
private:
  Q_DISABLE_COPY(QCPSelectionDecorator)
  friend class QCPAbstractPlottable;
};
Q_DECLARE_METATYPE(QCPSelectionDecorator*)


#endif // QCPSELECTIONDECORATOR_H
