#include "CQBezierPointItem.h"
#include <QGraphicsPathItem>
#include <QBrush>
#include <QPen>

// constructor
CQBezierPointItem::CQBezierPointItem(QGraphicsPathItem* pathItem, int index, const QPointF& pos)
    : QGraphicsEllipseItem(0, 0, 8, 8),
    mPathItem(pathItem),
    mIndex(index)
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

        // set path for element with mIndex
        path.setElementPositionAt(mIndex, newPos.x(), newPos.y());

        // apply curve changes
        mPathItem->setPath(path);
    }

    return QGraphicsEllipseItem::itemChange(change, value);
}

// getter function for mIndex
int CQBezierPointItem::getIndex() const
{
  return mIndex;
}
