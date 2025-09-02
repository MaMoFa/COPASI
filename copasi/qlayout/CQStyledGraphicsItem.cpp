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

#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QMenu>
#include <QAction>
#include <copasi/layout/CLReactionGlyph.h>
#include <QGraphicsSceneContextMenuEvent>

#include <copasi/qlayout/CQStyledGraphicsItem.h>
#include <copasi/qlayout/CQRenderConverter.h>
#include <copasi/qlayout/CQLayoutScene.h>

#include "copasi/copasi.h"

#include <copasi/layout/CLGlyphs.h>
#include <copasi/layout/CLRenderResolver.h>
#include "copasi/core/CRootContainer.h"

CQStyledGraphicsItem::CQStyledGraphicsItem(const CLGraphicalObject* go, const CLRenderResolver* resolver)
  : CQCopasiGraphicsItem(resolver, resolver != NULL ? resolver->resolveStyle(go) : NULL)
  , mWasMoved(false)
  , mpGraphicalObject(go)
{
  setFlag(QGraphicsItem::ItemIsMovable);
  setFlag(QGraphicsItem::ItemIsSelectable);
  setFlag(QGraphicsItem::ItemSendsGeometryChanges);
  
  setAcceptHoverEvents(true);
  setAcceptedMouseButtons(Qt::RightButton);
  setData(COPASI_LAYOUT_KEY, QString(go->getKey().c_str()));

  QString type;
  mLocked = go->isLocked();
  setData(Qt::UserRole + 1, type);

  QString type2;
  mSplit = go->isSplit();
  setData(Qt::UserRole + 1, type2);

  CQRenderConverter::fillGroupFromStyle(this, &go->getBoundingBox(), mpStyle, mpResolver);
}

CQStyledGraphicsItem::~CQStyledGraphicsItem()
{
}

void CQStyledGraphicsItem::contextMenuEvent(QGraphicsSceneContextMenuEvent * event)
{
  QMenu menu;
  QAction * lockAction = nullptr;
  QAction * splitAction = nullptr;

  lockAction = menu.addAction(mLocked ? "Unlock" : "Lock");

  CQLayoutScene * scene = dynamic_cast< CQLayoutScene * >(this->scene());
  CLMetabGlyph * pMetabGlyph = const_cast<CLMetabGlyph *>(dynamic_cast<const CLMetabGlyph *>(mpGraphicalObject));
  if (scene && pMetabGlyph && scene->canSplit(pMetabGlyph))
    {
      splitAction = menu.addAction(mSplit ? "Merge" : "Split");
    }
 
  QAction * selectedAction = menu.exec(event->screenPos());

  if (selectedAction == lockAction)
    {
      setLocked(!mLocked);    
    }
  else if (splitAction && selectedAction == splitAction)
  {
      setSplit(!mSplit);
  }

  event->accept();}

void CQStyledGraphicsItem::setLocked(bool locked)
{
  mLocked = locked;
  CQLayoutScene * currentScene = dynamic_cast< CQLayoutScene * >(scene());
  QString key = data(COPASI_LAYOUT_KEY).toString();
  currentScene->updateLock(key, mLocked);
  setFlag(QGraphicsItem::ItemIsMovable, !mLocked);
  update();
}

void CQStyledGraphicsItem::setSplit(bool split)
{
  mSplit = split;
  CQLayoutScene * currentScene = dynamic_cast< CQLayoutScene * >(scene());
  QString key = data(COPASI_LAYOUT_KEY).toString();
  currentScene->updateSplit(key, mSplit);
  update();
}


void CQStyledGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  CQLayoutScene * currentScene = dynamic_cast<CQLayoutScene *>(scene());

  if (mWasMoved && currentScene)
    {
      QPointF currentPos = pos();
      currentScene->updatePosition(data(COPASI_LAYOUT_KEY).toString(), currentPos);
      mWasMoved = false;
    }
  else
    {
      QGraphicsItem::mouseReleaseEvent(event);
    }
}

QVariant CQStyledGraphicsItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
  CQLayoutScene * currentScene = dynamic_cast<CQLayoutScene *>(scene());

  if (change == ItemPositionHasChanged && currentScene)
    {
      mWasMoved = true;
    }

  return QGraphicsItem::itemChange(change, value);
}

void CQStyledGraphicsItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
  if (mLocked)
    {
      painter->save();

      QPen redPen(Qt::red, 2);
      redPen.setStyle(Qt::DashLine);
      painter->setPen(redPen);
      painter->setBrush(Qt::NoBrush);

      QRectF rect = boundingRect();
      painter->drawRect(rect);

      painter->restore();
    }

  if (isSelected())
    {
      painter->save();
      QPen bluePen(Qt::blue, 2);
      bluePen.setStyle(Qt::DashLine);
      painter->setPen(bluePen);
      painter->setBrush(Qt::NoBrush);
      QRectF rect = boundingRect();
      painter->drawRect(rect);
      painter->restore();
    }
}