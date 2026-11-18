/* QCustomPlot 2.1.1 split source
   Copyright (C) 2011-2022 Emanuel Eichhammer
   Licensed under the GNU General Public License v3, see GPL.txt.
   Automatically split from the amalgamated qcustomplot.h/.cpp; keep code in sync. */

#ifndef QCPPOLARLEGENDITEM_H
#define QCPPOLARLEGENDITEM_H

#include "QCPGlobal.h"
#include "QCPPainter.h"
#include "QCPAbstractLegendItem.h"
#include "QCPLegend.h"

class QCP_LIB_DECL QCPPolarLegendItem : public QCPAbstractLegendItem
{
  Q_OBJECT
public:
  QCPPolarLegendItem(QCPLegend *parent, QCPPolarGraph *graph);
  
  // getters:
  QCPPolarGraph *polarGraph() { return mPolarGraph; }
  
protected:
  // property members:
  QCPPolarGraph *mPolarGraph;
  
  // reimplemented virtual methods:
  virtual void draw(QCPPainter *painter) Q_DECL_OVERRIDE;
  virtual QSize minimumOuterSizeHint() const Q_DECL_OVERRIDE;
  
  // non-virtual methods:
  QPen getIconBorderPen() const;
  QColor getTextColor() const;
  QFont getFont() const;
};


#endif // QCPPOLARLEGENDITEM_H
