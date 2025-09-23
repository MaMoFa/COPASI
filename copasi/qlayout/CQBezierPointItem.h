#pragma once

#include <QGraphicsEllipseItem>
#include <QPainterPath>
#include <QVariant>

class QGraphicsPathItem;
class CLPoint;

class CQBezierPointItem : public QGraphicsEllipseItem
{
public:
    // constructor: pathItem = curve, index = element index within the QPainterPath
  CQBezierPointItem(QGraphicsPathItem * pathItem, CLPoint * modelPoint, const QPointF & pos);
    // getter function for mIndex
    int getIndex() const;
    QPointF getInitialPoint() const;
    CLPoint * getModelPoint() const;
    void setModelPoint(CLPoint * modelPoint);
    static QList< int > findPointIndices(const QPainterPath & path, const QPointF & targetPoint, qreal tolerance = 1e-9);

protected:
    // itemChange is calls if point is moved
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
    QGraphicsPathItem* mPathItem;  // controlled curve
    QPointF mInitialPoint;
    CLPoint * mpModelPoint = nullptr;
};
