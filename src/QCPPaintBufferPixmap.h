/* QCustomPlot 2.1.1 split source
   Copyright (C) 2011-2022 Emanuel Eichhammer
   Licensed under the GNU General Public License v3, see GPL.txt.
   Automatically split from the amalgamated qcustomplot.h/.cpp; keep code in sync. */

#ifndef QCPPAINTBUFFERPIXMAP_H
#define QCPPAINTBUFFERPIXMAP_H

#include "QCPGlobal.h"
#include "QCPPainter.h"
#include "QCPAbstractPaintBuffer.h"

class QCP_LIB_DECL QCPPaintBufferPixmap : public QCPAbstractPaintBuffer
{
public:
  explicit QCPPaintBufferPixmap(const QSize &size, double devicePixelRatio);
  virtual ~QCPPaintBufferPixmap() Q_DECL_OVERRIDE;
  
  // reimplemented virtual methods:
  virtual QCPPainter *startPainting() Q_DECL_OVERRIDE;
  virtual void draw(QCPPainter *painter) const Q_DECL_OVERRIDE;
  void clear(const QColor &color) Q_DECL_OVERRIDE;
  
protected:
  // non-property members:
  QPixmap mBuffer;
  
  // reimplemented virtual methods:
  virtual void reallocateBuffer() Q_DECL_OVERRIDE;
};


#endif // QCPPAINTBUFFERPIXMAP_H
