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

#ifndef QCONNECTION_GRAPHICS_ITEM
#define QCONNECTION_GRAPHICS_ITEM

#include <QGraphicsItemGroup>
#include <QPainterPath>
#include <QtCore/QSharedPointer>
#include <QStyleOptionGraphicsItem>
#include <copasi/layout/CLCurve.h>
#include <copasi/qlayout/CQCopasiGraphicsItem.h>
#include "CQBezierPointItem.h"

class CLGlyphWithCurve;
class CLStyle;
class CQConnectionGraphicsItem : public QObject, public CQCopasiGraphicsItem, public QGraphicsItemGroup
{
  Q_OBJECT
public:

    /**
     * @brief Locks or unlocks the connection.
     * When locked, the connection cannot be moved.
     * @param locked New lock state.
     */
  void setLocked(bool locked);

    /**
     * @brief Sets whether the bezier control points are visible.
     * @param show True to show control points, false to hide.
     */
  virtual void setShow(bool show);

    /**
     * @brief Creates the Bezier control points for the curve.
     * Adds CQBezierPointItem objects to the scene for each relevant
     * control point in the curve.
     */
  void createBezierPointItem();

    /**
     * @brief Generates Bezier points from the current QPainterPath.
     */
  void createBezierPointsFromPainterPath();

    /**
     * @brief Maps the created Bezier points to the underlying model curve. (once it works)
     */
  void mapBezierPointsToModelCurve();

    /**
     * @brief Removes all Bezier control points from the scene.
     * Deletes all CQBezierPointItem objects associated with this connection.
     */
  void removeAllBezierPoints();

    /**
     * @brief Returns a reference to the list of Bezier points.
     * @return Vector of pointers to CQBezierPointItem objects.
     */
  std::vector< CQBezierPointItem * > & getBezierPoints();

    /**
     * @brief Returns the first QGraphicsPathItem child of this item.
     * @return Pointer to the path item or nullptr if none exists.
     */
  QGraphicsPathItem * getPathItem() const;

  CQConnectionGraphicsItem(const CLGlyphWithCurve * glyph, const CLRenderResolver * resolver = NULL);
  virtual ~CQConnectionGraphicsItem();
  static QSharedPointer<QPainterPath> getPath(const CLCurve& curve);
  virtual QPainterPath shape() const;
  void setUseFullShape(bool useFullShape);
protected:

    /**
     * @brief Paints the connection graphics item.
     * 
     * This method first calls the base class paint function to draw all child
     * items (lines and paths) of the connection. If the connection is locked,
     * it overlays the curves and lines in red to visually indicate the locked
     * state.
     * @param painter The QPainter used to render the item.
     * @param option  Provides style options for the item (e.g., selection state).
     * @param widget  Optional widget on which the item is being painted (may be nullptr).
     */
  virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option = new QStyleOptionGraphicsItem(), QWidget * widget = 0);

    /**
     * @brief Handles context menu events (right-click).
     * Provides actions for locking/unlocking connections and showing/hiding control points.
     */
  void contextMenuEvent(QGraphicsSceneContextMenuEvent * event);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
  virtual QVariant itemChange(GraphicsItemChange change, const QVariant & value);
  bool mWasMoved;
  bool mUseFullShape;
  QPainterPath mShape;
  QPainterPath mFullShape;

private:
  std::vector< CQBezierPointItem * > mBezierPoints; ///< List of bezier control points
  // std::vector< std::pair< QGraphicsPathItem *, const CLCurve * > > mCurveItems; ///< Map of path items to model curves
};

#endif
