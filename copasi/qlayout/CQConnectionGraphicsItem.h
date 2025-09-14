// Copyright (C) 2019 by Pedro Mendes, Rector and Visitors of the
// University of Virginia, University of Heidelberg, and University
// of Connecticut School of Medicine.
// All rights reserved.

// Copyright (C) 2017 - 2018 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and University of
// of Connecticut School of Medicine.
// All rights reserved.

// Copyright (C) 2013 - 2016 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

#ifndef QCONNECTION_GRAPHICS_ITEM
#define QCONNECTION_GRAPHICS_ITEM

#include <QGraphicsItemGroup>
#include <QPainterPath>
#include <QtCore/QSharedPointer>
#include <QStyleOptionGraphicsItem>
#include <copasi/layout/CLCurve.h>
#include <copasi/qlayout/CQCopasiGraphicsItem.h>
#include "CQBezierPointItem.h"

class CLGlyphWithCurve;
class CLStyle;
class CQConnectionGraphicsItem : public QObject, public CQCopasiGraphicsItem, public QGraphicsItemGroup
{
  Q_OBJECT
public:
  QGraphicsPathItem * getPathItem() const; // function to get the QGraphicsPathItem representing the connection
  void setLocked(bool locked); // function to set the lock status of the connection
  virtual void setShow(bool show); // function to set the state whether to show control points of the connection
  void createBezierPointItem();    // function to create control points for the bezier curve
  std::vector< CQBezierPointItem * > & getBezierPoints(); // function to get the list of control points for the bezier curve
  //void setBezierPoints(const std::vector< CQBezierPointItem * > & points); // function to set the list of control points for the bezier curve (not used)
  CQConnectionGraphicsItem(const CLGlyphWithCurve * glyph, const CLRenderResolver * resolver = NULL);
  virtual ~CQConnectionGraphicsItem();
  static QSharedPointer<QPainterPath> getPath(const CLCurve& curve);
  virtual QPainterPath shape() const;
  void setUseFullShape(bool useFullShape);
protected:
  virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option = new QStyleOptionGraphicsItem(), QWidget * widget = 0); // paint function for lock status
  void contextMenuEvent(QGraphicsSceneContextMenuEvent * event); // right click menu
  void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
  virtual QVariant itemChange(GraphicsItemChange change, const QVariant & value); // value is new position
  bool mWasMoved;
  bool mUseFullShape;
  QPainterPath mShape;
  QPainterPath mFullShape;

private:
  std::vector< CQBezierPointItem * > mBezierPoints; // list of control points for the bezier curve
};

#endif
