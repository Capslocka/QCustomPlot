/* QCustomPlot 2.1.1 split source
   Copyright (C) 2011-2022 Emanuel Eichhammer
   Licensed under the GNU General Public License v3, see GPL.txt.
   Automatically split from the amalgamated qcustomplot.h/.cpp; keep code in sync. */

#ifndef QCPPAINTBUFFERGLPBUFFER_H
#define QCPPAINTBUFFERGLPBUFFER_H

#include "QCPGlobal.h"
#include "QCPPainter.h"
#include "QCPAbstractPaintBuffer.h"

#ifdef QCP_OPENGL_PBUFFER
class QCP_LIB_DECL QCPPaintBufferGlPbuffer : public QCPAbstractPaintBuffer
{
public:
  explicit QCPPaintBufferGlPbuffer(const QSize &size, double devicePixelRatio, int multisamples);
  virtual ~QCPPaintBufferGlPbuffer() Q_DECL_OVERRIDE;
  
  // reimplemented virtual methods:
  virtual QCPPainter *startPainting() Q_DECL_OVERRIDE;
  virtual void draw(QCPPainter *painter) const Q_DECL_OVERRIDE;
  void clear(const QColor &color) Q_DECL_OVERRIDE;
  
protected:
  // non-property members:
  QGLPixelBuffer *mGlPBuffer;
  int mMultisamples;
  
  // reimplemented virtual methods:
  virtual void reallocateBuffer() Q_DECL_OVERRIDE;
};
#endif // QCP_OPENGL_PBUFFER


#endif // QCPPAINTBUFFERGLPBUFFER_H
