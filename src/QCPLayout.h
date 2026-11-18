/* QCustomPlot 2.1.1 split source
   Copyright (C) 2011-2022 Emanuel Eichhammer
   Licensed under the GNU General Public License v3, see GPL.txt.
   Automatically split from the amalgamated qcustomplot.h/.cpp; keep code in sync. */

#ifndef QCPLAYOUT_H
#define QCPLAYOUT_H

#include "QCPGlobal.h"
#include "QCPLayoutElement.h"

class QCP_LIB_DECL QCPLayout : public QCPLayoutElement
{
  Q_OBJECT
public:
  explicit QCPLayout();
  
  // reimplemented virtual methods:
  virtual void update(UpdatePhase phase) Q_DECL_OVERRIDE;
  virtual QList<QCPLayoutElement*> elements(bool recursive) const Q_DECL_OVERRIDE;
  
  // introduced virtual methods:
  virtual int elementCount() const = 0;
  virtual QCPLayoutElement* elementAt(int index) const = 0;
  virtual QCPLayoutElement* takeAt(int index) = 0;
  virtual bool take(QCPLayoutElement* element) = 0;
  virtual void simplify();
  
  // non-virtual methods:
  bool removeAt(int index);
  bool remove(QCPLayoutElement* element);
  void clear();
  
protected:
  // introduced virtual methods:
  virtual void updateLayout();
  
  // non-virtual methods:
  void sizeConstraintsChanged() const;
  void adoptElement(QCPLayoutElement *el);
  void releaseElement(QCPLayoutElement *el);
  QVector<int> getSectionSizes(QVector<int> maxSizes, QVector<int> minSizes, QVector<double> stretchFactors, int totalSize) const;
  static QSize getFinalMinimumOuterSize(const QCPLayoutElement *el);
  static QSize getFinalMaximumOuterSize(const QCPLayoutElement *el);
  
private:
  Q_DISABLE_COPY(QCPLayout)
  friend class QCPLayoutElement;
};


#endif // QCPLAYOUT_H
