// Copyright (C) 2017 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and University of
// of Connecticut School of Medicine.
// All rights reserved.

// Copyright (C) 2013 - 2016 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

#ifndef QSTYLED_GRAPHICS_ITEM
#define QSTYLED_GRAPHICS_ITEM

#include <QGraphicsItemGroup>

#include "copasi/qlayout/CQCopasiGraphicsItem.h"

class CLGraphicalObject;
class CLStyle;
class CLRenderResolver;
class QMouseEvent;
class CQStyledGraphicsItem : public QObject, public CQCopasiGraphicsItem, public QGraphicsItemGroup
{
  Q_OBJECT
public:

  /**
   * @brief Enumeration of possible lock states for a selection.
   *
   * - AllLocked: All selected items are locked.  
   * - AllUnlocked: All selected items are unlocked.  
   * - Mixed: Selection contains both locked and unlocked items.  
   */
  enum class LockState
  {
    AllLocked,
    AllUnlocked,
    Mixed
  };
  CQStyledGraphicsItem(const CLGraphicalObject * go, const CLRenderResolver * resolver = NULL);
  virtual ~CQStyledGraphicsItem();

  /**
   * @brief Set the lock status of this graphical object.
   * A locked object cannot be moved. The change is also propagated
   * to the associated scene.
   * @param locked True = locked, False = unlocked.
   */
  void setLocked(bool locked);

   /**
   * @brief Set the split status of this graphical object.
   * A metabolite glyph can be split into several parts if it
   * participates in multiple reactions. The change is also propagated
   * to the scene.
   * @param split True = split, False = do nothing.
   */
  void setSplit(bool split);

protected:

  /**
   * @brief Handle context menu events.
   * Creates a context menu with options such as lock/unlock,
   * split, merge, and delete, depending on the type and state
   * of the selected items.
   */
  void contextMenuEvent(QGraphicsSceneContextMenuEvent * event);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
  void mousePressEvent(QGraphicsSceneMouseEvent * event);
  virtual QVariant itemChange(GraphicsItemChange change, const QVariant & value);

  /**
   * @brief Custom painting of selection/lock markers.
   * - Red dashed rectangle: the item is locked.  
   * - Blue dashed rectangle: the item is selected.  
   * @param painter The QPainter used for drawing.
   * @param option  Style options for the item.
   * @param widget  The widget being painted on.
   */
  void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget) override;

  bool mWasMoved;

private:
  const CLGraphicalObject * mpGraphicalObject; ///< Pointer to the associated graphical object (metabolite glyph).
};

#endif
