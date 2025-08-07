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
#include <QBrush>
#include <QMenu>
#include <QAction>
#include <QGraphicsSceneContextMenuEvent>


#include <copasi/qlayout/CQConnectionGraphicsItem.h>
#include <copasi/qlayout/CQStyledGraphicsItem.h>
#include <copasi/qlayout/CQRenderConverter.h>
#include <copasi/qlayout/CQLayoutScene.h>
#include <copasi/layout/CLGlyphs.h>
#include <copasi/layout/CLReactionGlyph.h>
#include <copasi/layout/CLRenderResolver.h>

QSharedPointer<QPainterPath> CQConnectionGraphicsItem::getPath(const CLCurve& curve)
{
  QSharedPointer<QPainterPath> result = QSharedPointer<QPainterPath>(new QPainterPath());

  for (size_t i = 0; i < curve.getNumCurveSegments(); ++i)
    {
      const CLLineSegment* segment = curve.getSegmentAt(i);
      QPainterPath path = QPainterPath(
                            QPointF(
                              segment->getStart().getX(),
                              segment->getStart().getY()));

      if (segment->isBezier())
        {
          path.cubicTo(
            segment->getBase1().getX(), segment->getBase1().getY(),
            segment->getBase2().getX(), segment->getBase2().getY(),
            segment->getEnd().getX(), segment->getEnd().getY()
          );
        }
      else
        {
          path.lineTo(segment->getEnd().getX(), segment->getEnd().getY());
        }

      result->addPath(path);
    }

  return result;
}

QPainterPath CQConnectionGraphicsItem::shape() const
{
  if (mUseFullShape)
    return mFullShape;

  return mShape;
}

void
CQConnectionGraphicsItem::setUseFullShape(bool useFullShape)
{
  mUseFullShape = useFullShape;
}

void CQConnectionGraphicsItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
  painter->save();

  QGraphicsItemGroup::paint(painter, option, widget);

  painter->restore();

  if (this->isLocked())
    {
      painter->save();
      QColor Red(255, 0, 0);
      QPen redPen(Red, 4);
      painter->setPen(redPen);

      QList< QGraphicsItem * > stack = this->childItems();

      while (!stack.isEmpty())
        {
          QGraphicsItem * item = stack.takeLast();

          if (auto pathItem = dynamic_cast< QGraphicsPathItem * >(item))
            {
              painter->drawPath(pathItem->path());
            }
          else if (auto lineItem = dynamic_cast< QGraphicsLineItem * >(item))
            {
              painter->drawLine(lineItem->line());
            }

          const auto children = item->childItems();
          for (QGraphicsItem * child : children)
            {
              stack.append(child);
            }
        }
      painter->restore();
    }

  if (this->isShow())
    {
      painter->save();
      QColor pointColor(0, 128, 255);
      QBrush brush(pointColor);
      QPen pointPen(pointColor);
      pointPen.setWidth(1);
      painter->setPen(pointPen);
      painter->setBrush(brush);

      const qreal radius = 4.0;

      QList< QGraphicsItem * > stack = this->childItems();

      while (!stack.isEmpty())
        {
          QGraphicsItem * item = stack.takeLast();

          if (auto pathItem = dynamic_cast< QGraphicsPathItem * >(item))
            {
              const QPainterPath path = pathItem->path();

              for (int i = 0; i < path.elementCount(); ++i)
                {
                  const QPainterPath::Element e = path.elementAt(i);
                  QPointF point(e.x, e.y);
                  painter->drawEllipse(point, radius, radius);

                  //if (e.type == QPainterPath::CurveToDataElement)
                  //{
                  //    QPointF point(e.x, e.y);
                  //    painter->drawEllipse(point, radius, radius);
                  //}
                }
            }

          const auto children = item->childItems();
          for (QGraphicsItem * child : children)
            {
              stack.append(child);
            }
        }

      painter->restore();
    }
}

void CQConnectionGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  CQLayoutScene * currentScene = dynamic_cast<CQLayoutScene *>(scene());

  if (mWasMoved && currentScene)
    {
      QPointF currentPos = pos();
      currentScene->updatePosition(data(COPASI_LAYOUT_KEY).toString(), currentPos);
      mWasMoved = false;
      mUseFullShape = false;
    }
  else
    {
      QGraphicsItem::mouseReleaseEvent(event);
    }
}

QVariant CQConnectionGraphicsItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
  CQLayoutScene * currentScene = dynamic_cast<CQLayoutScene *>(scene());

  if (change == ItemPositionHasChanged && currentScene)
    {
      mWasMoved = true;
    }

  return QGraphicsItem::itemChange(change, value);
}

CQConnectionGraphicsItem::CQConnectionGraphicsItem(const CLGlyphWithCurve* curveGlyph, const CLRenderResolver* resolver)
  : CQCopasiGraphicsItem(resolver, resolver != NULL ? resolver->resolveStyle(curveGlyph) : NULL)
  , mWasMoved(false)
  , mUseFullShape(false)
{
  setFlag(QGraphicsItem::ItemIsMovable);
  setFlag(QGraphicsItem::ItemSendsGeometryChanges);
  setAcceptHoverEvents(true);
  setAcceptedMouseButtons(Qt::RightButton);

  QString type;
  setData(Qt::UserRole + 1, type);

  setData(COPASI_LAYOUT_KEY, QString(curveGlyph->getKey().c_str()));
  mLocked = curveGlyph->isLocked();
  mShow = curveGlyph->isShow();

  QSharedPointer<QPainterPath> path = getPath(curveGlyph->getCurve());
  mShape.addPath(*path);
  mFullShape.addPath(*path);
  QGraphicsPathItem* item;
  QGraphicsItemGroup* itemGroup;

  if (curveGlyph->getCurve().getNumCurveSegments() > 0)
    {
      item = new QGraphicsPathItem(*path);
      itemGroup = new QGraphicsItemGroup();
      itemGroup->addToGroup(item);

      if (mpStyle != NULL)
        CQRenderConverter::applyStyle(item, &curveGlyph->getBoundingBox(), mpStyle->getGroup(), resolver, itemGroup);

      addToGroup(itemGroup);
    }

  const CLReactionGlyph* reaction = dynamic_cast<const CLReactionGlyph*>(curveGlyph);

  if (reaction != NULL)
    {
      const CDataVector<CLMetabReferenceGlyph> & list = reaction->getListOfMetabReferenceGlyphs();

      for (CDataVector<CLMetabReferenceGlyph>::const_iterator it = list.begin(); it != list.end(); ++it)
        {
          const CLMetabReferenceGlyph* metab = it;
          const CLStyle *style = resolver->resolveStyle(metab);

          if (metab->getCurve().getNumCurveSegments() > 0)
            {
              path = getPath(metab->getCurve());
              mFullShape.addPath(*path);

              item = new QGraphicsPathItem(*path);
              itemGroup = new QGraphicsItemGroup();
              itemGroup->addToGroup(item);
              CQRenderConverter::applyStyle(item, &metab->getBoundingBox(), style == NULL ?
                                            mpStyle == NULL ? NULL :  mpStyle->getGroup()
                                              : style->getGroup(), resolver, itemGroup);
              addToGroup(itemGroup);
            }
        }
    }

  const CLGeneralGlyph* general = dynamic_cast<const CLGeneralGlyph*>(curveGlyph);

  if (general != NULL)
    {
      const CDataVector<CLReferenceGlyph> & list = general->getListOfReferenceGlyphs();

      for (CDataVector<CLReferenceGlyph>::const_iterator it = list.begin(); it != list.end(); ++it)
        {
          const CLReferenceGlyph* glyph = it;
          const CLStyle *style = resolver->resolveStyle(glyph);

          if (glyph ->getCurve().getNumCurveSegments() > 0)
            {
              path = getPath(glyph ->getCurve());
              mFullShape.addPath(*path);

              item = new QGraphicsPathItem(*path);
              itemGroup = new QGraphicsItemGroup();
              itemGroup->addToGroup(item);
              CQRenderConverter::applyStyle(item, &glyph ->getBoundingBox(), style == NULL ?
                                            mpStyle == NULL ? NULL :  mpStyle->getGroup()
                                              : style->getGroup(), resolver, itemGroup);
              addToGroup(itemGroup);
            }
        }
    }

  // add decorations on reactions
  itemGroup = new QGraphicsItemGroup();
  CQRenderConverter::fillGroupFromStyle(itemGroup, &curveGlyph->getBoundingBox(), mpStyle, resolver);

  if (itemGroup->childItems().size() > 0)
    addToGroup(itemGroup);
  else
    delete itemGroup;
}

void CQConnectionGraphicsItem::contextMenuEvent(QGraphicsSceneContextMenuEvent * event)
{
  QMenu menu;
  QAction * lockAction = nullptr;
  QAction * showAction = nullptr;

  lockAction = menu.addAction(mLocked ? "Unlock" : "Lock");
  showAction = menu.addAction(mShow ? "Hide" : "Show");

  QAction * selectedAction = menu.exec(event->screenPos());

  if (selectedAction == lockAction)
    {
      setLocked(!mLocked);
    }
  if (selectedAction == showAction)
  {
      setShow(!mShow);
  }

  event->accept();
}

void CQConnectionGraphicsItem::setLocked(bool locked)
  {
  mLocked = locked;
  setFlag(QGraphicsItem::ItemIsMovable, !mLocked);
  CQLayoutScene * currentScene = dynamic_cast< CQLayoutScene * >(scene());
  currentScene->updateLock(data(COPASI_LAYOUT_KEY).toString(), mLocked);
  update();
}

void CQConnectionGraphicsItem::setShow(bool show)
{
  mShow = show;
  CQLayoutScene * currentScene = dynamic_cast< CQLayoutScene * >(scene());
  currentScene->updateShow(data(COPASI_LAYOUT_KEY).toString(), mShow);
  update();
}


CQConnectionGraphicsItem::~CQConnectionGraphicsItem()
{
}
