#pragma once

#include <QGraphicsEllipseItem>
#include <QPainterPath>
#include <QVariant>

class QGraphicsPathItem;

class CQBezierPointItem : public QGraphicsEllipseItem
{
public:
    // constructor: pathItem = curve, index = element index within the QPainterPath
    CQBezierPointItem(QGraphicsPathItem* pathItem, const QPointF& pos);
    // getter function for mIndex
    int getIndex() const;
    QPointF getInitialPoint() const;
    static QList< int > findPointIndices(const QPainterPath & path, const QPointF & targetPoint, qreal tolerance = 1e-9);

protected:
    // itemChange is calls if point is moved
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
    QGraphicsPathItem* mPathItem;  // controlled curve
    QPointF mInitialPoint;
};
