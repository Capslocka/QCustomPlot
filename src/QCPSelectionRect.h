/* QCustomPlot 2.1.1 split source
   Copyright (C) 2011-2022 Emanuel Eichhammer
   Licensed under the GNU General Public License v3, see GPL.txt.
   Automatically split from the amalgamated qcustomplot.h/.cpp; keep code in sync. */

#ifndef QCPSELECTIONRECT_H
#define QCPSELECTIONRECT_H

#include "QCPGlobal.h"
#include "QCPPainter.h"
#include "QCPLayerable.h"
#include "QCPRange.h"

class QCP_LIB_DECL QCPSelectionRect : public QCPLayerable
{
  Q_OBJECT
public:
  explicit QCPSelectionRect(QCustomPlot *parentPlot);
  virtual ~QCPSelectionRect() Q_DECL_OVERRIDE;
  
  // getters:
  QRect rect() const { return mRect; }
  QCPRange range(const QCPAxis *axis) const;
  QPen pen() const { return mPen; }
  QBrush brush() const { return mBrush; }
  bool isActive() const { return mActive; }
  
  // setters:
  void setPen(const QPen &pen);
  void setBrush(const QBrush &brush);
  
  // non-property methods:
  Q_SLOT void cancel();
  
signals:
  void started(QMouseEvent *event);
  void changed(const QRect &rect, QMouseEvent *event);
  void canceled(const QRect &rect, QInputEvent *event);
  void accepted(const QRect &rect, QMouseEvent *event);
  
protected:
  // property members:
  QRect mRect;
  QPen mPen;
  QBrush mBrush;
  // non-property members:
  bool mActive;
  
  // introduced virtual methods:
  virtual void startSelection(QMouseEvent *event);
  virtual void moveSelection(QMouseEvent *event);
  virtual void endSelection(QMouseEvent *event);
  virtual void keyPressEvent(QKeyEvent *event);
  
  // reimplemented virtual methods
  virtual void applyDefaultAntialiasingHint(QCPPainter *painter) const Q_DECL_OVERRIDE;
  virtual void draw(QCPPainter *painter) Q_DECL_OVERRIDE;
  
  friend class QCustomPlot;
};


#endif // QCPSELECTIONRECT_H
