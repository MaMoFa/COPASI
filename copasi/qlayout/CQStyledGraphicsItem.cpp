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

CQStyledGraphicsItem::CQStyledGraphicsItem(const CLGraphicalObject* go, const CLRenderResolver* resolver)
  : CQCopasiGraphicsItem(resolver, resolver != NULL ? resolver->resolveStyle(go) : NULL)
  , mWasMoved(false)
{
  setFlag(QGraphicsItem::ItemIsMovable);
  setFlag(QGraphicsItem::ItemIsSelectable);
  setFlag(QGraphicsItem::ItemSendsGeometryChanges);
  setAcceptHoverEvents(true);
  setAcceptedMouseButtons(Qt::RightButton);
  setData(COPASI_LAYOUT_KEY, QString(go->getKey().c_str()));

  QString type;
  //if (dynamic_cast< const CLReactionGlyph * >(go))
  //  type = "reaction";
  //else if (dynamic_cast< const CLMetabGlyph * >(go))
  //  type = "species";
  setData(Qt::UserRole + 1, type);

  CQRenderConverter::fillGroupFromStyle(this, &go->getBoundingBox(), mpStyle, mpResolver);
 /* for (QGraphicsItem * child : childItems())
    {
      child->setData(Qt::UserRole + 1, type);
    }*/
}

CQStyledGraphicsItem::~CQStyledGraphicsItem()
{
}

void CQStyledGraphicsItem::contextMenuEvent(QGraphicsSceneContextMenuEvent * event)
{
  QGraphicsItem * parent = parentItem();
  if (parent)
    {
      // Delegiere an das Eltern-Item (z. B. CQStyledGraphicsItem)
      // QApplication::sendEvent(parent, event);
    }
   QString type = data(Qt::UserRole + 1).toString();
  qDebug() << "Kontextmenü für Typ:" << type;
  qDebug() << "Item bounding rect: " << boundingRect();
  qDebug() << "Right click received at:" << event->scenePos();

  QMenu menu;
  QAction * lockAction = nullptr;

  if (type == "species" || type == "reaction")
    {
      lockAction = menu.addAction(mLocked ? "Unlock" : "Lock");
    }
  else
    {
      menu.addAction("Kein Kontextmenü für Typ: " + type);
    }

  QAction * selectedAction = menu.exec(event->screenPos());

  if (selectedAction == lockAction)
    {
      mLocked = !mLocked;
      update(); // optional visuelles Feedback
      qDebug() << (mLocked ? "Gesperrt" : "Entsperrt");
    }

  event->accept();
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
