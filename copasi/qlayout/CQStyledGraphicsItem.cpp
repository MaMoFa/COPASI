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

#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QMenu>
#include <QAction>
#include <copasi/layout/CLReactionGlyph.h>
#include <QGraphicsSceneContextMenuEvent>

#include <copasi/qlayout/CQStyledGraphicsItem.h>
#include <copasi/qlayout/CQRenderConverter.h>
#include <copasi/qlayout/CQLayoutScene.h>

#include "copasi/copasi.h"

#include <copasi/layout/CLGlyphs.h>
#include <copasi/layout/CLRenderResolver.h>
#include "copasi/core/CRootContainer.h"

CQStyledGraphicsItem::CQStyledGraphicsItem(const CLGraphicalObject* go, const CLRenderResolver* resolver)
  : CQCopasiGraphicsItem(resolver, resolver != NULL ? resolver->resolveStyle(go) : NULL)
  , mWasMoved(false)
  , mpGraphicalObject(go)
{
  setFlag(QGraphicsItem::ItemIsMovable);
  setFlag(QGraphicsItem::ItemIsSelectable);
  setFlag(QGraphicsItem::ItemSendsGeometryChanges);
  
  setAcceptHoverEvents(true);
  setAcceptedMouseButtons(Qt::RightButton);
  setData(COPASI_LAYOUT_KEY, QString(go->getKey().c_str()));

  QString type;
  mLocked = go->isLocked(); // if true the object cannot be moved in the editor
  setData(Qt::UserRole + 1, type); // store lock status

  QString type2;
  mSplit = go->isSplit(); // if true the metabolite glyph is split into n parts where n is the number of participations in reactions
  setData(Qt::UserRole + 1, type2); // store split status

  CQRenderConverter::fillGroupFromStyle(this, &go->getBoundingBox(), mpStyle, mpResolver);
}

CQStyledGraphicsItem::~CQStyledGraphicsItem()
{
}

// Enumeration for the lock state of a set of selected items
enum class LockState
{
  AllLocked,
  AllUnlocked,
  Mixed
};

// Context menu for Lock/Split/Merge/Delete
void CQStyledGraphicsItem::contextMenuEvent(QGraphicsSceneContextMenuEvent * event)
{
  CQLayoutScene * scene = dynamic_cast< CQLayoutScene * >(this->scene());
  if (!scene)
    return;

  // Update selection: if clicked item is not selected, select only it
  if (!this->isSelected())
    {
      scene->clearSelection();
      this->setSelected(true);
    }

  const QList< QGraphicsItem * > sel = scene->selectedItems();
  if (sel.empty())
    return;

  QMenu menu; // Create context menu

  // Determine LockState
  bool hasLocked = false; // At least one selected item is locked
  bool hasUnlocked = false; // At least one selected item is unlocked
  LockState lockState = LockState::AllLocked; // Initial assumption

  // Check lock status of selected items
  for (QGraphicsItem * gi : sel)
    {
      // Only CQStyledGraphicsItem have lock status
      if (auto * styled = dynamic_cast< CQStyledGraphicsItem * >(gi))
        {
          if (styled->mLocked)
            hasLocked = true; // At least one item is locked
          else
            hasUnlocked = true; // At least one item is unlocked

          if (hasLocked && hasUnlocked)
            {
              lockState = LockState::Mixed; // Mixed lock status
              break;
            }
        }
    }

  if (!(hasLocked && hasUnlocked))
    {
      lockState = hasLocked ? LockState::AllLocked : LockState::AllUnlocked; // All locked or all unlocked
    }

  // Add Lock / Unlock / Invert actions
  QAction * invertAction = nullptr;
  QAction * lockAction = nullptr;
  QAction * unlockAction = nullptr;

  // Add actions according to lock state
  switch (lockState)
    {
    case LockState::AllLocked:
      unlockAction = menu.addAction("Unlock");
      break;
    case LockState::AllUnlocked:
      lockAction = menu.addAction("Lock");
      break;
    case LockState::Mixed:
      invertAction = menu.addAction("Invert");
      lockAction = menu.addAction("Lock");
      unlockAction = menu.addAction("Unlock");
      break;
    }

  // Check if any selected item is splittable
  bool anySplittable = false;
  // iterate over selected items
  for (QGraphicsItem * gi : sel)
    {
      // iterate over selected items
      if (auto * styled = dynamic_cast< CQStyledGraphicsItem * >(gi))
        {
          // Only CLMetabGlyph can be split
          const CLMetabGlyph * metabGlyph = dynamic_cast< const CLMetabGlyph * >(styled->mpGraphicalObject); // Only metabolites can be split
          if (metabGlyph && scene->canSplit(metabGlyph))                                                     // Check if splittable
            {
              anySplittable = true;
              break;
            }
        }
    }

  QAction * splitAction = anySplittable ? menu.addAction("Split") : nullptr; // Add Split action if any selected item is splittable
  QAction * mergeAction = scene->canMerge() ? menu.addAction("Merge") : nullptr; // Add Merge action if any selected items can be merged

  // Check if any selected item is a Metabolite glyph for deletion
  bool anyMetabSelected = false;
  // iterate over selected items
  for (QGraphicsItem * gi : sel)
    {
      // iterate over selected items
      if (auto * styled = dynamic_cast< CQStyledGraphicsItem * >(gi))
        {
          const CLMetabGlyph * metabGlyph = dynamic_cast< const CLMetabGlyph * >(styled->mpGraphicalObject); // Only metabolites can be deleted with associated reaction glyphs
          if (metabGlyph)
            {
              anyMetabSelected = true;
              break;
            }
        }
    }
  QAction * deleteAction = anyMetabSelected ? menu.addAction("Delete") : nullptr; // Add Delete action if any selected item is a Metabolite glyph

  // Execute the menu
  QAction * selectedAction = menu.exec(event->screenPos());

  if (!selectedAction)
    return; // Do nothing if user canceled menu

  // Lock/Unlock/Invert actions
  // Invert lock status of selected items
  if (selectedAction == invertAction)
    {
      // Iterate over selected items
      for (QGraphicsItem * gi : sel)
        {
          if (auto * styled = dynamic_cast< CQStyledGraphicsItem * >(gi))
            styled->setLocked(!styled->mLocked); // Invert lock status
        }
    }
  // Lock selected items
  else if (selectedAction == lockAction)
    {
      // Iterate over selected items
      for (QGraphicsItem * gi : sel)
        {
          if (auto * styled = dynamic_cast< CQStyledGraphicsItem * >(gi))
            styled->setLocked(true); // Lock items
        }
    }
  // Unlock selected items
  else if (selectedAction == unlockAction)
    {
      // Iterate over selected items
      for (QGraphicsItem * gi : sel)
        {
          if (auto * styled = dynamic_cast< CQStyledGraphicsItem * >(gi))
            styled->setLocked(false); // Unlock items
        }
    }

  // Split selected splittable items
  else if (splitAction && selectedAction == splitAction)
    {
      // Iterate over selected items
      for (QGraphicsItem * gi : sel)
        {
          if (auto * styled = dynamic_cast< CQStyledGraphicsItem * >(gi))
            {
              const CLMetabGlyph * metabGlyph = dynamic_cast< const CLMetabGlyph * >(styled->mpGraphicalObject); // Only metabolites can be split
              if (metabGlyph && scene->canSplit(metabGlyph))
                scene->updateSplit(QString::fromStdString(metabGlyph->getKey()), true); // Split metab glyph
            }
        }
    }

  // Merge selected mergable items
  else if (mergeAction && selectedAction == mergeAction)
    {
      scene->mergeSelected();
    }
 
  // Delete selected glyphs and all associated reaction glyphs
  else if (deleteAction && selectedAction == deleteAction)
    {
      for (QGraphicsItem * gi : sel)
        {
          if (auto * styled = dynamic_cast< CQStyledGraphicsItem * >(gi))
            {
              const CLMetabGlyph * metabGlyph = dynamic_cast< const CLMetabGlyph * >(styled->mpGraphicalObject);
              if (metabGlyph)
                {
                  const CMetab * metab = dynamic_cast< const CMetab * >(metabGlyph->getModelObject());
                  if (metab)
                    scene->removeMetab(metab); // Remove glyph + all associated reaction glyphs
                }
            }
        }
    }
  event->accept();
}

// Set lock status and update scene and item flags
void CQStyledGraphicsItem::setLocked(bool locked)
{
  mLocked = locked;
  CQLayoutScene * currentScene = dynamic_cast< CQLayoutScene * >(scene());
  QString key = data(COPASI_LAYOUT_KEY).toString();
  currentScene->updateLock(key, mLocked);
  setFlag(QGraphicsItem::ItemIsMovable, !mLocked);
  update();
}

// Set split status and update scene
void CQStyledGraphicsItem::setSplit(bool split)
{
  mSplit = split;
  CQLayoutScene * currentScene = dynamic_cast< CQLayoutScene * >(scene());
  QString key = data(COPASI_LAYOUT_KEY).toString();
  currentScene->updateSplit(key, mSplit);
  update();
}


void CQStyledGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  CQLayoutScene * currentScene = dynamic_cast<CQLayoutScene *>(scene());

  if (mWasMoved && currentScene)
    {
      QPointF currentPos = pos();
      currentScene->updatePosition(data(COPASI_LAYOUT_KEY).toString(), currentPos);
      mWasMoved = false;
    }
  else
    {
      QGraphicsItem::mouseReleaseEvent(event);
    }
}

QVariant CQStyledGraphicsItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
  CQLayoutScene * currentScene = dynamic_cast<CQLayoutScene *>(scene());

  if (change == ItemPositionHasChanged && currentScene)
    {
      mWasMoved = true;
    }

  return QGraphicsItem::itemChange(change, value);
}

void CQStyledGraphicsItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
  if (mLocked)
    {
      painter->save();

      QPen redPen(Qt::red, 2);
      redPen.setStyle(Qt::DashLine);
      painter->setPen(redPen);
      painter->setBrush(Qt::NoBrush);

      QRectF rect = boundingRect();
      painter->drawRect(rect);

      painter->restore();
    }

  if (isSelected())
    {
      painter->save();
      QPen bluePen(Qt::blue, 2);
      bluePen.setStyle(Qt::DashLine);
      painter->setPen(bluePen);
      painter->setBrush(Qt::NoBrush);
      QRectF rect = boundingRect();
      painter->drawRect(rect);
      painter->restore();
    }
}

// Handle mouse press to manage selection correctly for right-clicks
void CQStyledGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
  // Only handle right mouse button
  if (event->button() == Qt::RightButton)
    {
      CQLayoutScene * scene = dynamic_cast< CQLayoutScene * >(this->scene());
      if (!scene)
        return;

      // If right-clicked item is NOT selected, deselect all others and select only this item
      if (!this->isSelected())
        {
          scene->clearSelection();
          this->setSelected(true);
        }
      // If already selected, keep selection unchanged

      // Accept event to prevent default Qt selection behavior
      event->accept();
      return;
    }

  // For left-click and others, use default behavior
  QGraphicsItem::mousePressEvent(event);
}
