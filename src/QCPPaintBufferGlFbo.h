/* QCustomPlot 2.1.1 split source
   Copyright (C) 2011-2022 Emanuel Eichhammer
   Licensed under the GNU General Public License v3, see GPL.txt.
   Automatically split from the amalgamated qcustomplot.h/.cpp; keep code in sync. */

#ifndef QCPPAINTBUFFERGLFBO_H
#define QCPPAINTBUFFERGLFBO_H

#include "QCPGlobal.h"
#include "QCPPainter.h"
#include "QCPAbstractPaintBuffer.h"

#ifdef QCP_OPENGL_FBO
class QCP_LIB_DECL QCPPaintBufferGlFbo : public QCPAbstractPaintBuffer
{
public:
  explicit QCPPaintBufferGlFbo(const QSize &size, double devicePixelRatio, QWeakPointer<QOpenGLContext> glContext, QWeakPointer<QOpenGLPaintDevice> glPaintDevice);
  virtual ~QCPPaintBufferGlFbo() Q_DECL_OVERRIDE;
  
  // reimplemented virtual methods:
  virtual QCPPainter *startPainting() Q_DECL_OVERRIDE;
  virtual void donePainting() Q_DECL_OVERRIDE;
  virtual void draw(QCPPainter *painter) const Q_DECL_OVERRIDE;
  void clear(const QColor &color) Q_DECL_OVERRIDE;
  
protected:
  // non-property members:
  QWeakPointer<QOpenGLContext> mGlContext;
  QWeakPointer<QOpenGLPaintDevice> mGlPaintDevice;
  QOpenGLFramebufferObject *mGlFrameBuffer;
  
  // reimplemented virtual methods:
  virtual void reallocateBuffer() Q_DECL_OVERRIDE;
};
#endif // QCP_OPENGL_FBO


#endif // QCPPAINTBUFFERGLFBO_H
