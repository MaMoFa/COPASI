#include "CQBezierPointItem.h"
#include <QGraphicsPathItem>
#include <QBrush>
#include <QPen>

// constructor
CQBezierPointItem::CQBezierPointItem(QGraphicsPathItem* pathItem, const QPointF& pos)
  : QGraphicsEllipseItem(0, 0, 8, 8)
  , mPathItem(pathItem)
  , mInitialPoint(pos)
{
    setPos(pos);

    // color, brush, pen
    setBrush(QBrush(QColor(0, 128, 255)));
    setPen(QPen(Qt::NoPen));

    // flags: selectable, movable, sends geometrical changes
    setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsGeometryChanges);
}

QVariant CQBezierPointItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemPositionChange && mPathItem)
    {
        // update curve
        QPainterPath path = mPathItem->path();
        QPointF newPos = value.toPointF();
        auto indices = findPointIndices(path, mInitialPoint);
        if (indices.empty())
          return QGraphicsEllipseItem::itemChange(change, value);
        
        // set path for all indices
        for (auto index : indices)
          {
            path.setElementPositionAt(index, newPos.x(), newPos.y());
          }
        
        // apply curve changes
        mPathItem->setPath(path);

        // remember new pos
        mInitialPoint = newPos;
    }

    return QGraphicsEllipseItem::itemChange(change, value);
}

QList< int > CQBezierPointItem::findPointIndices(const QPainterPath & path, const QPointF & targetPoint, qreal tolerance /*= 1e-9*/)
{
  QList< int > indices;
  for (int i = 0; i < path.elementCount(); ++i)
    {
      QPainterPath::Element elem = path.elementAt(i);
      QPointF point(elem.x, elem.y);
      if (QLineF(point, targetPoint).length() < tolerance)
        {
          indices.append(i);
        }
    }
  return indices; // Not found
}

// getter function for mIndex
int CQBezierPointItem::getIndex() const
{
  auto indices = findPointIndices(mPathItem->path(), mInitialPoint);
  if (indices.empty())
    return -1;
  return indices.first();
}

QPointF CQBezierPointItem::getInitialPoint() const
{
  return mInitialPoint;
}
