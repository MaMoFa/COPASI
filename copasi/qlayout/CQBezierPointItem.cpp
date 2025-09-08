#include "CQBezierPointItem.h"
#include <QGraphicsPathItem>
#include <QBrush>
#include <QPen>

CQBezierPointItem::CQBezierPointItem(QGraphicsPathItem* pathItem, int index, const QPointF& pos, QGraphicsItem* parent)
    : QGraphicsEllipseItem(-4, -4, 8, 8, parent), // Ellipse zentriert um pos
    mPathItem(pathItem),
    mIndex(index)
{
    setPos(pos);

    // Farbe / Brush / Pen
    setBrush(QBrush(QColor(0, 128, 255)));
    setPen(QPen(Qt::NoPen));

    // Flags: selektierbar, verschiebbar, meldet GeometryChanges
    setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsGeometryChanges);

    // Optional: Parent darf nicht selektierbar sein, sonst blockiert es Children
    if (parent)
        parent->setFlag(QGraphicsItem::ItemIsSelectable, false);
}

QVariant CQBezierPointItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemPositionChange && mPathItem)
    {
        // Kurve aktualisieren
        QPainterPath path = mPathItem->path();
        QPointF newPos = value.toPointF();

        // Element am Index setzen
        path.setElementPositionAt(mIndex, newPos.x(), newPos.y());

        // Kurve übernehmen
        mPathItem->setPath(path);
    }

    return QGraphicsEllipseItem::itemChange(change, value);
}
