#pragma once

#include <QGraphicsEllipseItem>
#include <QPainterPath>
#include <QVariant>

class QGraphicsPathItem;

class CQBezierPointItem : public QGraphicsEllipseItem
{
public:
    // Konstruktor: pathItem = die Kurve, index = Position im QPainterPath
    CQBezierPointItem(QGraphicsPathItem* pathItem, int index, const QPointF& pos, QGraphicsItem* parent = nullptr);

protected:
    // itemChange wird aufgerufen, wenn der Punkt bewegt wird
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
    QGraphicsPathItem* mPathItem;  // Die Kurve, die gesteuert wird
    int mIndex;                     // Index im QPainterPath
};
