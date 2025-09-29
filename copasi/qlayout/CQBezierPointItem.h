#pragma once

#include <QGraphicsEllipseItem>
#include <QPainterPath>
#include <QVariant>

class QGraphicsPathItem;
class CLPoint;

/**
 * @class CQBezierPointItem
 * @brief Represents a draggable point on a Bezier curve.
 *
 * This class provides a visual handle for a point on a QPainterPath.
 * Moving the point updates both the QGraphicsPathItem and the associated
 * CLPoint in the underlying model (once it works). The item is selectable and movable
 * within the LayoutScene.
 */
class CQBezierPointItem : public QGraphicsEllipseItem
{
public:

    /**
     * @brief Constructs a CQBezierPointItem.
     *
     * Initializes a small ellipse representing a point on a curve.
     * Sets the initial position, brush, pen, and flags to make the item
     * selectable, movable, and responsive to geometry changes.
     *
     * @param pathItem    Pointer to the QGraphicsPathItem representing the curve.
     * @param modelPoint  Pointer to the underlying CLPoint model object.
     * @param pos         Initial position of the point in scene coordinates.
     */
  CQBezierPointItem(QGraphicsPathItem * pathItem, CLPoint * modelPoint, const QPointF & pos);

    /**
     * @brief Returns the index of this point in the QPainterPath.
     *
     * If the point is not found in the path, returns -1.
     *
     * @return Index of the point within the path, or -1 if not found.
     */
    //int getIndex() const;

    /**
     * @brief Returns the initial position of the point.
     *
     * @return QPointF representing the initial position of this point.
     */
  QPointF getInitialPoint() const;

    /**
     * @brief Returns the pointer to the underlying CLPoint model.
     *
     * @return Pointer to the CLPoint associated with this point item.
     */
  //CLPoint * getModelPoint() const;

    /**
     * @brief Sets the underlying CLPoint model for this item.
     *
     * @param modelPoint Pointer to the new CLPoint model object.
     */
    //void setModelPoint(CLPoint * modelPoint);

    /**
     * @brief Finds all indices of points in a QPainterPath that match a target point.
     *
     * Compares points using a specified tolerance and returns all matching indices.
     *
     * @param path         The QPainterPath to search.
     * @param targetPoint  The point to find within the path.
     * @param tolerance    Maximum distance to consider points equal (default 1e-9).
     * @return QList<int> containing indices of matching points.
     */
    //static QList< int > findPointIndices(const QPainterPath & path, const QPointF & targetPoint, qreal tolerance = 1e-9);

protected:
    /**
     * @brief Handles item changes, e.g., when the point is moved.
     *
     * Updates the underlying CLPoint model (once it works) and modifies the corresponding
     * element(s) of the QPainterPath to reflect the new position.
     *
     * @param change The type of change (e.g., ItemPositionChange).
     * @param value  The new value associated with the change.
     * @return The resulting value after handling the change.
     */
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
    QGraphicsPathItem* mPathItem; ///< Controlled curve in the scene
    QPointF mInitialPoint; ///< Last known position of the point
    //CLPoint * mpModelPoint = nullptr;///< Associated model point
};
