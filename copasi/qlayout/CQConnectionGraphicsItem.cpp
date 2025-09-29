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

#include "CQBezierPointItem.h"
#include <copasi/layout/CLCurve.h>
#include "copasi/core/CRootContainer.h"
#include "copasi/report/CKeyFactory.h"

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

// function to paint curves red if locked
void CQConnectionGraphicsItem::paint(QPainter * painter,
                                     const QStyleOptionGraphicsItem * option,
                                     QWidget * widget)
{
  painter->save(); // save painter

  // paint child items (lines, paths)
  QGraphicsItemGroup::paint(painter, option, widget);

  painter->restore(); // restore painter

  //paint red if locked
  if (this->isLocked())
    {
      painter->save();
      QColor Red(255, 0, 0);
      QPen redPen(Red, 4);
      painter->setPen(redPen);

      // paint all child paths and lines
      QList< QGraphicsItem * > stack = this->childItems();
      while (!stack.isEmpty())
        {
          QGraphicsItem * item = stack.takeLast();

          if (auto pathItem = dynamic_cast< QGraphicsPathItem * >(item))
            painter->drawPath(pathItem->path()); // draw the path
          else if (auto lineItem = dynamic_cast< QGraphicsLineItem * >(item))
            painter->drawLine(lineItem->line()); // draw the line
          
          const auto children = item->childItems();
          for (QGraphicsItem * child : children)
            stack.append(child); // add child items to stack to process them as well
        }

      painter->restore(); // restore painter
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
  mLocked = curveGlyph->isLocked(); // initial lock status
  mShow = curveGlyph->isShow();     // initial show status

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
      // mCurveItems.emplace_back(item, &curveGlyph->getCurve());
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
              // mCurveItems.emplace_back(item, &curveGlyph->getCurve());
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
              // mCurveItems.emplace_back(item, &curveGlyph->getCurve());
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

// right click menu
void CQConnectionGraphicsItem::contextMenuEvent(QGraphicsSceneContextMenuEvent * event)
{
  // create context menu
  QMenu menu;
  QAction * lockAction = nullptr; // to lock/unlock the item
  QAction * showAction = nullptr; // to show/hide the bezier control points

  lockAction = menu.addAction(mLocked ? "Unlock" : "Lock"); // change lock status
  showAction = menu.addAction(mShow ? "Hide" : "Show");     // change show status

  QAction * selectedAction = menu.exec(event->screenPos()); // execute menu

  // process action selected
  if (selectedAction == lockAction)
    {
      setLocked(!mLocked); // toggle lock status
    }
  if (selectedAction == showAction)
    {
      setShow(!mShow); // toggle show status
      createBezierPointItem(); // create bezier control points if show is true
    }

  event->accept(); // mark event as handled
}

// function to set whether to show the control points of a reaction glyph
void CQConnectionGraphicsItem::setShow(bool show)
{
  CQCopasiGraphicsItem::setShow(show);                                     // set show status in base class
  mShow = show;                                                            // set show status
  CQLayoutScene * currentScene = dynamic_cast< CQLayoutScene * >(scene()); // get the scene
  if (currentScene)                                                        // check whether scene is valid
    currentScene->updateShow(data(COPASI_LAYOUT_KEY).toString(), mShow); // update the show status in the layout
  update();
}

// function to set whether the item is locked
void CQConnectionGraphicsItem::setLocked(bool locked)
  {
  mLocked = locked;                                                        // set lock status
    setFlag(QGraphicsItem::ItemIsMovable, !mLocked);                         // set movable flag according to lock status
  CQLayoutScene * currentScene = dynamic_cast< CQLayoutScene * >(scene()); // get the scene
    currentScene->updateLock(data(COPASI_LAYOUT_KEY).toString(), mLocked);   // update the lock status in the layout
  update();                                                                // update the item (to show red border if locked)
  }

CQConnectionGraphicsItem::~CQConnectionGraphicsItem()
{
}

// function to create the bezier control points
void CQConnectionGraphicsItem::createBezierPointItem()
{
  if (this->isShow())
    {
      createBezierPointsFromPainterPath();
      //mapBezierPointsToModelCurve();
    }
  // if show is false, remove all bezier points
  if (!this->isShow())
  {
      removeAllBezierPoints();
  }
}

void CQConnectionGraphicsItem::removeAllBezierPoints()
{
  if (!this->isLocked())
    {
      this->setFlag(QGraphicsItem::ItemIsSelectable, true); // set curve selectable
      this->setFlag(QGraphicsItem::ItemIsMovable, true);    // set curve movable
    }
  // remove all bezier points from scene
  for (auto * bp : mBezierPoints)
    {
      if (bp && scene())
        {
          scene()->removeItem(bp);
        }
    }
  mBezierPoints.clear(); // clear the list of bezier points
  return;
}

//void CQConnectionGraphicsItem::mapBezierPointsToModelCurve()
//{
//  for (auto & [item, curve] : mCurveItems)
//    {
//      if (curve == nullptr)
//        continue;
//
//      for (size_t k = 0; k < curve->getNumCurveSegments(); ++k)
//        {
//          const CLLineSegment * seg = curve->getSegmentAt(k);
//          if (!seg->isBezier())
//            continue;
//
//          for (auto * bp : mBezierPoints)
//            {
//              QPointF point = bp->getInitialPoint();
//
//              auto match = [&](const CLPoint & cp) -> bool {
//                return QLineF(point, QPointF(cp.getX(), cp.getY())).length() < 1e-6;
//              };
//
//              if (match(seg->getBase1()))
//                {
//                  bp->setModelPoint(&const_cast< CLPoint & >(seg->getBase1()));
//                }
//              else if (match(seg->getBase2()))
//                {
//                  bp->setModelPoint(&const_cast< CLPoint & >(seg->getBase2()));
//                }
//            }
//        }
//    }
//}

void CQConnectionGraphicsItem::createBezierPointsFromPainterPath()
{
  QList< QGraphicsItem * > stack = this->childItems();
  // iterate over all child items
  while (!stack.isEmpty())
    {
      QGraphicsItem * item = stack.takeLast(); // get last item

      // check whether item is a path item
      if (auto * pathItem = dynamic_cast< QGraphicsPathItem * >(item))
        {
          const QPainterPath itemPath = pathItem->path(); // get the path
          const QPainterPath path = mFullShape;

          // iterate over all elements in the path
          QPainterPath::ElementType lastType = QPainterPath::MoveToElement;
          for (int i = 0; i < path.elementCount(); ++i)
            {
              QPainterPath::Element elem = path.elementAt(i); // get the element
              QPointF point(elem.x, elem.y);                  // get the point

              // Only control points, we gotta skip the 2nd curve to data, as it is the endpoint!
              // controlpoint 1 = QPainterPath::CurveToElement
              // controlpoint 2 = QPainterPath::CurveToDataElement
              // endPoint = QPainterPath::CurveToDataElement

              if (lastType == QPainterPath::CurveToDataElement && elem.type == QPainterPath::CurveToDataElement)
                {
                  continue;
                }

              // for now only allow to move control points of bezier curves
              if (elem.type == QPainterPath::CurveToElement || elem.type == QPainterPath::CurveToDataElement)
                {

                  int index = -1;
                  for (int j = 0; j < itemPath.elementCount(); ++j)
                    {
                      auto current = itemPath.elementAt(j);
                      if (current.type != elem.type)
                        continue;
                      if (QLineF(point, QPointF(current.x, current.y)).length() < 1e-9)
                        {
                          index = j;
                          break;
                        }
                    }

                  if (index < 0)
                    continue;

                  bool exist = false;
                  for (auto * bp : mBezierPoints)
                    {
                      if (bp->getInitialPoint() == point)
                        {
                          exist = true;
                          break;
                        }
                    }

                  if (exist)
                    continue;

                  // create point if it does not exist
                  auto * bp = new CQBezierPointItem(pathItem, nullptr, point);    // create bezier point item
                  bp->setFlag(QGraphicsItem::ItemIsMovable, true);                // set movable
                  bp->setZValue(1000);                                            // set z value to be on top of other items

                  mBezierPoints.push_back(bp); // add to list of bezier points

                  // set curve un-selectable and not movable
                  this->setFlag(QGraphicsItem::ItemIsSelectable, false);
                  this->setFlag(QGraphicsItem::ItemIsMovable, false);

                  scene()->addItem(bp); // add to scene
                }

              lastType = elem.type;
            }
        }

      // iterate over children
      const auto children = item->childItems();
      for (QGraphicsItem * child : children)
        stack.append(child); // add children to stack
    }
}

QGraphicsPathItem * CQConnectionGraphicsItem::getPathItem() const
{
  for (auto * child : this->childItems())
    {
      if (auto * pathItem = dynamic_cast< QGraphicsPathItem * >(child))
        return pathItem;
    }
  return nullptr; // if no pathItem 
}

std::vector< CQBezierPointItem * > & CQConnectionGraphicsItem::getBezierPoints()
{
  return mBezierPoints;
}