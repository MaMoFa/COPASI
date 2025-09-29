// Copyright (C) 2019 - 2020 by Pedro Mendes, Rector and Visitors of the
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

#ifndef QLAYOUT_SCENE_H
#define QLAYOUT_SCENE_H

#include <QGraphicsScene>
#include <QtCore/QSharedPointer>

#include <string>

#include "copasi/model/CMetab.h"
#include "copasi/layout/CLayout.h"

class CLayout;
class CLGraphicalObject;
class CLRenderResolver;
class CDataModel;
class CLRenderInformationBase;
class CQCopasiEffect;
class CCopasiSpringLayout;

class CQLayoutScene: public QGraphicsScene
{
  Q_OBJECT
public:
  CQLayoutScene(CLayout *layout, CDataModel* model = NULL, CLRenderInformationBase* renderInformation = NULL);
  virtual ~CQLayoutScene();
  void setResolver(CLRenderResolver* resolver);
  const CLRenderResolver* getResolver() const;
  CLRenderResolver * getResolver();
  void saveToFile(const std::string& fileName, const std::string& fileType = "pdf");
  void setLayout(CLayout *layout, CDataModel* model = NULL, CLRenderInformationBase* renderInformation = NULL);
  void setRenderInformation(CDataModel* model, CLRenderInformationBase* renderInformation);
  QGraphicsItem* getItemFor(const std::string& cn);
  const CLayout* getCurrentLayout() const;
  CLayout* getCurrentLayout();
  const CLRenderInformationBase* getCurrentRenderInfo() const;
  void updatePosition(const QString& key, const QPointF& newPos);

  /**
   * @brief Update the lock status of a graphical object in the layout.
   * Sets the @c locked flag of the corresponding @c CLGraphicalObject.
   * Locked items cannot be moved, and are rendered with a red overlay.
   * @param key    Unique key of the graphical object (COPASI key).
   * @param locked True to lock the object, false to unlock it.
   */
  void updateLock(const QString & key, bool locked);

  /**
   * @brief Update the "show control points" state of a reaction glyph.
   * When enabled, bezier control points of the glyph are made visible
   * in the scene for interactive editing.
   * @param key  Unique key of the reaction glyph.
   * @param show True to show control points, false to hide them.
   */
  void updateShow(const QString & kay, bool show);

  /**
   * @brief Update the split status of a metabolite glyph.
   * If @p split is true, the original glyph is replaced by side metabolite
   * glyphs in the layout. Otherwise, the glyph is kept as a single node.
   * @param key   Unique key of the metabolite glyph.
   * @param split True to split the glyph, false to do nothing.
   */
  void updateSplit(const QString & key, bool split);

  /**
   * @brief Remove a metabolite and its associated glyphs from the layout.
   * This includes metabolite glyphs, text glyphs, and references in reaction glyphs.
   * @param pMetab Pointer to the metabolite to be removed.
   */
  void removeMetab(const CMetab * pMetab);

  /**
   * @brief Check whether a metabolite glyph can be split.
   * A glyph is splittable if it participates in multiple reactions.
   * @param pMetabGlyph Pointer to the glyph to test.
   * @return True if the glyph can be split, false otherwise.
   */
  bool canSplit(const CLMetabGlyph * pMetabGlyph) const;

  /**
   * @brief Check whether the selected metabolite glyphs can be merged.
   * A merge is possible if at least two selected glyphs represent
   * the same species.
   * @return True if the current selection is mergable, false otherwise.
   */
  bool canMerge() const;
  public slots:

  /**
   * @brief Merge selected metabolite glyphs of the same species.
   * Glyphs are combined into a single representative glyph, references
   * in reaction glyphs are updated, and redundant glyphs/text glyphs
   * are removed.
   * Emits @c recreateNeeded() after completion.
   */
  void mergeSelected();
  void recreate();
signals:
  void recreateNeeded();
protected:
  void initializeResolver(CDataModel* model, CLRenderInformationBase* renderInformation);
private:
  void addGlyph(const CLGraphicalObject* go);
  void fillFromLayout(const CLayout* layout);
  CCopasiSpringLayout * mpSpringLayout; ///< Layout algorithm for positioning glyphs

  CLayout* mpLayout;
  CLRenderInformationBase* mpRender;
  QSharedPointer<CLRenderResolver> mpResolver;

  std::map< std::string, QGraphicsItem*> mItems;
};

#endif
