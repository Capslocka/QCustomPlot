/* QCustomPlot 2.1.1 split source
   Copyright (C) 2011-2022 Emanuel Eichhammer
   Licensed under the GNU General Public License v3, see GPL.txt.
   Automatically split from the amalgamated qcustomplot.h/.cpp; keep code in sync. */

#ifndef QCPABSTRACTPAINTBUFFER_H
#define QCPABSTRACTPAINTBUFFER_H

#include "QCPGlobal.h"
#include "QCPPainter.h"

class QCP_LIB_DECL QCPAbstractPaintBuffer
{
public:
  explicit QCPAbstractPaintBuffer(const QSize &size, double devicePixelRatio);
  virtual ~QCPAbstractPaintBuffer();
  
  // getters:
  QSize size() const { return mSize; }
  bool invalidated() const { return mInvalidated; }
  double devicePixelRatio() const { return mDevicePixelRatio; }
  
  // setters:
  void setSize(const QSize &size);
  void setInvalidated(bool invalidated=true);
  void setDevicePixelRatio(double ratio);
  
  // introduced virtual methods:
  virtual QCPPainter *startPainting() = 0;
  virtual void donePainting() {}
  virtual void draw(QCPPainter *painter) const = 0;
  virtual void clear(const QColor &color) = 0;
  
protected:
  // property members:
  QSize mSize;
  double mDevicePixelRatio;
  
  // non-property members:
  bool mInvalidated;
  
  // introduced virtual methods:
  virtual void reallocateBuffer() = 0;
};


#endif // QCPABSTRACTPAINTBUFFER_H
