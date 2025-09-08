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
  mLocked = go->isLocked();
  setData(Qt::UserRole + 1, type);

  QString type2;
  mSplit = go->isSplit();
  setData(Qt::UserRole + 1, type2);

  CQRenderConverter::fillGroupFromStyle(this, &go->getBoundingBox(), mpStyle, mpResolver);
}

CQStyledGraphicsItem::~CQStyledGraphicsItem()
{
}

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

  QMenu menu;

  // Determine LockState
  bool hasLocked = false;
  bool hasUnlocked = false;
  LockState lockState = LockState::AllLocked;

  for (QGraphicsItem * gi : sel)
    {
      if (auto * styled = dynamic_cast< CQStyledGraphicsItem * >(gi))
        {
          if (styled->mLocked)
            hasLocked = true;
          else
            hasUnlocked = true;

          if (hasLocked && hasUnlocked)
            {
              lockState = LockState::Mixed;
              break;
            }
        }
    }

  if (!(hasLocked && hasUnlocked))
    {
      lockState = hasLocked ? LockState::AllLocked : LockState::AllUnlocked;
    }

  // Add Lock / Unlock / Invert actions
  QAction * invertAction = nullptr;
  QAction * lockAction = nullptr;
  QAction * unlockAction = nullptr;

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
  for (QGraphicsItem * gi : sel)
    {
      if (auto * styled = dynamic_cast< CQStyledGraphicsItem * >(gi))
        {
          const CLMetabGlyph * metabGlyph = dynamic_cast< const CLMetabGlyph * >(styled->mpGraphicalObject);
          if (metabGlyph && scene->canSplit(metabGlyph))
            {
              anySplittable = true;
              break;
            }
        }
    }

  QAction * splitAction = anySplittable ? menu.addAction("Split") : nullptr;
  QAction * mergeAction = scene->canMerge() ? menu.addAction("Merge") : nullptr;

  // Check if any selected item is a Metabolite glyph for deletion
  bool anyMetabSelected = false;
  for (QGraphicsItem * gi : sel)
    {
      if (auto * styled = dynamic_cast< CQStyledGraphicsItem * >(gi))
        {
          const CLMetabGlyph * metabGlyph = dynamic_cast< const CLMetabGlyph * >(styled->mpGraphicalObject);
          if (metabGlyph)
            {
              anyMetabSelected = true;
              break;
            }
        }
    }
  QAction * deleteAction = anyMetabSelected ? menu.addAction("Delete") : nullptr;

  // Execute the menu
  QAction * selectedAction = menu.exec(event->screenPos());

  if (!selectedAction)
    return; // Do nothing if user canceled menu

  // Lock/Unlock/Invert actions
  if (selectedAction == invertAction)
    {
      for (QGraphicsItem * gi : sel)
        {
          if (auto * styled = dynamic_cast< CQStyledGraphicsItem * >(gi))
            styled->setLocked(!styled->mLocked);
        }
    }
  else if (selectedAction == lockAction)
    {
      for (QGraphicsItem * gi : sel)
        {
          if (auto * styled = dynamic_cast< CQStyledGraphicsItem * >(gi))
            styled->setLocked(true);
        }
    }
  else if (selectedAction == unlockAction)
    {
      for (QGraphicsItem * gi : sel)
        {
          if (auto * styled = dynamic_cast< CQStyledGraphicsItem * >(gi))
            styled->setLocked(false);
        }
    }

  // Split selected splittable items
  else if (splitAction && selectedAction == splitAction)
    {
      for (QGraphicsItem * gi : sel)
        {
          if (auto * styled = dynamic_cast< CQStyledGraphicsItem * >(gi))
            {
              const CLMetabGlyph * metabGlyph = dynamic_cast< const CLMetabGlyph * >(styled->mpGraphicalObject);
              if (metabGlyph && scene->canSplit(metabGlyph))
                scene->updateSplit(QString::fromStdString(metabGlyph->getKey()), true);
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

void CQStyledGraphicsItem::setLocked(bool locked)
{
  mLocked = locked;
  CQLayoutScene * currentScene = dynamic_cast< CQLayoutScene * >(scene());
  QString key = data(COPASI_LAYOUT_KEY).toString();
  currentScene->updateLock(key, mLocked);
  setFlag(QGraphicsItem::ItemIsMovable, !mLocked);
  update();
}

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