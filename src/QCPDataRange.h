/* QCustomPlot 2.1.1 split source
   Copyright (C) 2011-2022 Emanuel Eichhammer
   Licensed under the GNU General Public License v3, see GPL.txt.
   Automatically split from the amalgamated qcustomplot.h/.cpp; keep code in sync. */

#ifndef QCPDATARANGE_H
#define QCPDATARANGE_H

#include "QCPGlobal.h"

class QCP_LIB_DECL QCPDataRange
{
public:
  QCPDataRange();
  QCPDataRange(int begin, int end);
  
  bool operator==(const QCPDataRange& other) const { return mBegin == other.mBegin && mEnd == other.mEnd; }
  bool operator!=(const QCPDataRange& other) const { return !(*this == other); }
  
  // getters:
  int begin() const { return mBegin; }
  int end() const { return mEnd; }
  int size() const { return mEnd-mBegin; }
  int length() const { return size(); }
  
  // setters:
  void setBegin(int begin) { mBegin = begin; }
  void setEnd(int end)  { mEnd = end; }
  
  // non-property methods:
  bool isValid() const { return (mEnd >= mBegin) && (mBegin >= 0); }
  bool isEmpty() const { return length() == 0; }
  QCPDataRange bounded(const QCPDataRange &other) const;
  QCPDataRange expanded(const QCPDataRange &other) const;
  QCPDataRange intersection(const QCPDataRange &other) const;
  QCPDataRange adjusted(int changeBegin, int changeEnd) const { return QCPDataRange(mBegin+changeBegin, mEnd+changeEnd); }
  bool intersects(const QCPDataRange &other) const;
  bool contains(const QCPDataRange &other) const;
  
private:
  // property members:
  int mBegin, mEnd;

};
Q_DECLARE_TYPEINFO(QCPDataRange, Q_MOVABLE_TYPE);


#endif // QCPDATARANGE_H
