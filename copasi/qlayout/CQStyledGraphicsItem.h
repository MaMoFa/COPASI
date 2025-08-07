// Copyright (C) 2017 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and University of
// of Connecticut School of Medicine.
// All rights reserved.

// Copyright (C) 2013 - 2016 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

#ifndef QSTYLED_GRAPHICS_ITEM
#define QSTYLED_GRAPHICS_ITEM

#include <QGraphicsItemGroup>

#include "copasi/qlayout/CQCopasiGraphicsItem.h"

class CLGraphicalObject;
class CLStyle;
class CLRenderResolver;
class QMouseEvent;
class CQStyledGraphicsItem : public QObject
  , public CQCopasiGraphicsItem
  , public QGraphicsItemGroup
{
  Q_OBJECT
public:
  CQStyledGraphicsItem(const CLGraphicalObject * go, const CLRenderResolver * resolver = NULL);
  virtual ~CQStyledGraphicsItem();

  void setLocked(bool locked);
  void setShow(bool show);

protected:
  void contextMenuEvent(QGraphicsSceneContextMenuEvent * event);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
  virtual QVariant itemChange(GraphicsItemChange change, const QVariant & value);
  void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget) override;

  bool mWasMoved;
};

#endif
