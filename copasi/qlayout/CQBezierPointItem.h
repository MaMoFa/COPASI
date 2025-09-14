#pragma once

#include <QGraphicsEllipseItem>
#include <QPainterPath>
#include <QVariant>

class QGraphicsPathItem;

class CQBezierPointItem : public QGraphicsEllipseItem
{
public:
    // constructor: pathItem = curve, index = element index within the QPainterPath
    CQBezierPointItem(QGraphicsPathItem* pathItem, int index, const QPointF& pos);
    // getter function for mIndex
    int getIndex() const;

protected:
    // itemChange is calles if point is moved
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
    QGraphicsPathItem* mPathItem;  // controlled curve
    int mIndex;                     // element index within the QPainterPath
};
