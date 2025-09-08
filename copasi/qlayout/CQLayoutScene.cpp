// Copyright (C) 2019 - 2024 by Pedro Mendes, Rector and Visitors of the
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

#include <QtCore/QCoreApplication>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QPainter>
#include <QPrinter>
#include <QSvgGenerator>
#include <QImage>
#include <QGraphicsEffect>

#include "copasi/copasi.h"

#include "copasi/qlayout/CQLayoutScene.h"
#include "copasi/qlayout/CQCopasiEffect.h"
#include "copasi/qlayout/CQLabelGraphicsItem.h"
#include "copasi/qlayout/CQStyledGraphicsItem.h"
#include "copasi/qlayout/CQConnectionGraphicsItem.h"
#include "copasi/qlayout/CQRenderConverter.h"
#include "copasi/layout/CLayout.h"
#include "copasi/layout/CLGlyphs.h"
#include "copasi/layout/CLText.h"
#include "copasi/layout/CLReactionGlyph.h"
#include "copasi/layout/CLRenderResolver.h"
#include "copasi/layout/CLGlobalRenderInformation.h"
#include "copasi/layout/CListOfLayouts.h"
#include "copasi/layout/CLLocalRenderInformation.h"
#include "copasi/layout/CLDefaultStyles.h"
#include "copasi/layout/CCopasiSpringLayout.h"

#include <copasi/model/CModel.h>
#include <copasi/model/CCompartment.h>
#include <copasi/model/CMetab.h>
#include <copasi/model/CReaction.h>
#include <copasi/model/CChemEq.h>
#include <copasi/utilities/CVersion.h>

#include "copasi/CopasiDataModel/CDataModel.h"
#include "copasi/core/CRootContainer.h"
#include "copasi/report/CKeyFactory.h"

CQLayoutScene::CQLayoutScene(CLayout* layout, CDataModel* model, CLRenderInformationBase* renderInformation)
  : QGraphicsScene()
  , mpLayout(layout)
  , mpRender(renderInformation)
  , mpResolver(NULL)
  , mpSpringLayout(nullptr)
{
  initializeResolver(model, renderInformation);
  connect(this, SIGNAL(recreateNeeded()), this, SLOT(recreate()), Qt::QueuedConnection);
  mpSpringLayout = new CCopasiSpringLayout(mpLayout);
}

void CQLayoutScene::setLayout(CLayout *layout, CDataModel* model, CLRenderInformationBase* renderInformation)
{
  mpLayout = layout;
  setRenderInformation(model, renderInformation);
}

void CQLayoutScene::setRenderInformation(CDataModel* model, CLRenderInformationBase* renderInformation)
{
  initializeResolver(model, renderInformation);
}

const CLayout* CQLayoutScene::getCurrentLayout() const
{
  return mpLayout;
}

CLayout* CQLayoutScene::getCurrentLayout()
{
  return mpLayout;
}

const CLRenderInformationBase* CQLayoutScene::getCurrentRenderInfo() const
{
  return mpRender;
}

void CQLayoutScene::saveToFile(const std::string& fileName, const std::string& fileType /*= "pdf"*/)
{
  if (fileType == "pdf")
    {
      QPrinter printer(QPrinter::HighResolution);
      printer.setOutputFormat(QPrinter::PdfFormat);
      printer.setOutputFileName(fileName.c_str());
      QPainter painter(&printer);
      painter.setRenderHints(
        QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
      render(&painter, QRect(), itemsBoundingRect());
      painter.end();
    }
  else if (fileType == "svg")
    {
      QRectF rect = itemsBoundingRect();
      QSvgGenerator generator;
      generator.setTitle(getCurrentLayout()->getObjectName().c_str());
      generator.setDescription(QString("Exported using COPASI: %1").arg(CVersion::VERSION.getVersion().c_str()));
      generator.setFileName(fileName.c_str());
      generator.setViewBox(rect);
      generator.setSize(QSize(rect.width(), rect.height()));
      QPainter painter;
      painter.begin(&generator);
      render(&painter);
      painter.end();
    }
  else
    {
      const int scale = 2;
      QImage image(QSize(width()*scale, height()*scale), QImage::Format_ARGB32);
      QPainter painter(&image);
      painter.setRenderHints(
        QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
      render(&painter, image.rect(), itemsBoundingRect());
      painter.end();
      image.save(fileName.c_str(), fileType.c_str());
    }
}

void CQLayoutScene::initializeResolver(CDataModel* model, CLRenderInformationBase* renderInformation)
{
  if (model == NULL)
    return;

  if (renderInformation == NULL)
    {
      if (mpLayout != NULL && mpLayout->getListOfLocalRenderInformationObjects().size() > 0)
        mpRender = &mpLayout->getListOfLocalRenderInformationObjects()[0];
      else if (model->getListOfLayouts()->getListOfGlobalRenderInformationObjects().size() > 0)
        mpRender = &model->getListOfLayouts()->getListOfGlobalRenderInformationObjects()[0];
      else
        mpRender = getDefaultStyle(0);
    }
  else
    mpRender = renderInformation;

  if (mpLayout == NULL || mpRender == NULL)
    return;

  CLLocalRenderInformation* local = dynamic_cast<CLLocalRenderInformation*>(mpRender);

  if (local != NULL)
    mpResolver = QSharedPointer<CLRenderResolver>(new CLRenderResolver(*local, mpLayout->getListOfLocalRenderInformationObjects(),   model->getListOfLayouts()->getListOfGlobalRenderInformationObjects()));
  else
    mpResolver = QSharedPointer<CLRenderResolver>(new CLRenderResolver(*dynamic_cast<CLGlobalRenderInformation*>(mpRender), model->getListOfLayouts()->getListOfGlobalRenderInformationObjects()));
}

void CQLayoutScene::setResolver(CLRenderResolver* resolver)
{
  mpResolver = QSharedPointer<CLRenderResolver>(resolver);
}

const CLRenderResolver* CQLayoutScene::getResolver() const
{
  return mpResolver.data();
}

CLRenderResolver * CQLayoutScene::getResolver()
{
  return mpResolver.data();
}

CQLayoutScene::~CQLayoutScene()
{
}

void CQLayoutScene::recreate()
{
  fillFromLayout(mpLayout);
  invalidate();
}

void CQLayoutScene::addGlyph(const CLGraphicalObject* go)
{
  if (go == NULL) return;

  const CLGlyphWithCurve* curveGlyph = dynamic_cast<const CLGlyphWithCurve*>(go);
  const CLReactionGlyph* reaction = dynamic_cast<const CLReactionGlyph*>(go);
  const CLTextGlyph* text = dynamic_cast<const CLTextGlyph*>(go);
  const CLGeneralGlyph* general = dynamic_cast<const CLGeneralGlyph*>(go);
  QGraphicsItem *item = NULL;

  if (curveGlyph != NULL)
    {
      if (curveGlyph->getCurve().getNumCurveSegments() > 0 || reaction != NULL || general != NULL)
        item = new CQConnectionGraphicsItem(curveGlyph,
                                            mpResolver == NULL ? NULL : mpResolver.data());
    }
  else if (text != NULL)
    {
      item = new CQLabelGraphicsItem(text, mpResolver == NULL ? NULL : mpResolver.data());
    }
  else
    {
      item = new CQStyledGraphicsItem(go, mpResolver == NULL ? NULL : mpResolver.data());
    }

  if (item != NULL && !dynamic_cast<CQCopasiGraphicsItem*>(item)->isValid())
    {
      delete item;
      item = NULL;
    }

  if (item != NULL)
    {
      QString cn = QString::fromStdString(go->getKey());

      CDataObject* obj = go->getModelObject();

      if (obj != NULL && text == NULL)
        {
          item->setData(COPASI_OBJECT_CN, QString(obj->getStringCN().c_str()));
          mItems[obj->getStringCN()] = item;
        }

      CQCopasiGraphicsItem * cItem = dynamic_cast< CQCopasiGraphicsItem * >(item);

      if (obj != NULL && text == NULL)
        {
          item->setData(COPASI_OBJECT_CN, QString(obj->getStringCN().c_str()));

          if (cItem)
            {
              cItem->setLocked(go->isLocked());
              cItem->setShow(go->isShow());

              QGraphicsItem * graphicsItem = dynamic_cast< QGraphicsItem * >(cItem);
              if (graphicsItem)
                {
                  QGraphicsItem::GraphicsItemFlags flags = graphicsItem->flags();

                  if (go->isLocked())
                    flags &= ~QGraphicsItem::ItemIsMovable; // Wenn locked, nicht beweglich
                  else
                    flags |= QGraphicsItem::ItemIsMovable; // Wenn unlocked, beweglich

                  graphicsItem->setFlags(flags);
                }
            }


          mItems[obj->getStringCN()] = item;
        }

      // if we have a z position, use it
      double z = go->getPosition().getZ();

      if (z == z && z != 0)
        item->setZValue(z);

      addItem(item);
    }

  if (general != NULL)
    {
      const CDataVector<CLGraphicalObject> & subGlyphs = general->getListOfSubglyphs();
      CDataVector<CLGraphicalObject>::const_iterator it = subGlyphs.begin();

      while (it != subGlyphs.end())
        {
          addGlyph(it);
          ++it;
        }
    }
}

QGraphicsItem* CQLayoutScene::getItemFor(const std::string& cn)
{
  return mItems[cn];
}

void CQLayoutScene::fillFromLayout(const CLayout* layout)
{
  if (layout == NULL) return;

  clear();
  mItems.clear();

  if (mpRender != NULL && mpResolver != NULL)
    {
      CQRenderConverter::setBackground(this, mpRender->getBackgroundColor(), mpResolver.data());
    }

  const CDataVector<CLCompartmentGlyph> & comps = layout->getListOfCompartmentGlyphs();

  CDataVector<CLCompartmentGlyph>::const_iterator itComp = comps.begin();

  while (itComp != comps.end())
    {
      addGlyph(itComp);
      ++itComp;
    }

  const CDataVector<CLReactionGlyph> & reactions = layout->getListOfReactionGlyphs();

  CDataVector<CLReactionGlyph>::const_iterator itReactions = reactions.begin();

  while (itReactions != reactions.end())
    {
      addGlyph(itReactions);
      ++itReactions;
    }

  const CDataVector<CLMetabGlyph> & species = layout->getListOfMetaboliteGlyphs();

  CDataVector<CLMetabGlyph>::const_iterator itSpecies = species.begin();

  while (itSpecies != species.end())
    {
      addGlyph(itSpecies);
      ++itSpecies;
    }

  const CDataVector<CLTextGlyph> & texts = layout->getListOfTextGlyphs();

  CDataVector<CLTextGlyph>::const_iterator itTexts = texts.begin();

  while (itTexts != texts.end())
    {
      addGlyph(itTexts);
      ++itTexts;
    }

  const CDataVector<CLGeneralGlyph> & list = layout->getListOfGeneralGlyphs();

  CDataVector<CLGeneralGlyph>::const_iterator itList = list.begin();

  while (itList != list.end())
    {
      addGlyph(itList);
      ++itList;
    }

}

CLGraphicalObject* getTextForItem(const CLayout* layout, const CLGraphicalObject* obj)
{
  const CDataVector<CLTextGlyph> & texts = layout->getListOfTextGlyphs();
  CDataVector<CLTextGlyph>::const_iterator it = texts.begin();

  while (it != texts.end())
    {
      if (it->getGraphicalObjectKey() == obj->getKey())
        return it.constCast();

      ++it;
    }

  return NULL;
}

CLGraphicalObject* getReactionGlyphForKey(const CLayout* layout, const std::string& key)
{
  const CDataVector<CLReactionGlyph> & reactions = layout->getListOfReactionGlyphs();
  CDataVector<CLReactionGlyph>::const_iterator it = reactions.begin();

  while (it != reactions.end())
    {
      if (it->getModelObjectKey() == key)
        return it.constCast();

      ++it;
    }

  return NULL;
}

CLGraphicalObject* getMetabGlyphForKey(const CLayout* layout, const CMetab* metab)
{
  const CDataVector<CLMetabGlyph> & metabs = layout->getListOfMetaboliteGlyphs();
  CDataVector<CLMetabGlyph>::const_iterator it = metabs.begin();

  while (it != metabs.end())
    {
      if (it->getModelObjectKey() == metab->getKey())
        return it.constCast();

      ++it;
    }

  return NULL;
}

void moveObject(CLGraphicalObject* obj, const CLPoint& delta, CLayout* layout)
{
  if (obj == NULL) return;

  CLReactionGlyph* pRG  = dynamic_cast<CLReactionGlyph*>(obj);

  if (pRG != NULL)
    {
      CLPoint position = pRG->getPosition();

      if (position.getX() == 0 && position.getY() == 0
          && pRG->getDimensions().getWidth() == 0
          && pRG->getDimensions().getHeight() == 0
          && pRG->getCurve().getNumCurveSegments() > 0)
        {
          position = pRG->getCurve().getCurveSegments()[0].getStart();
          pRG->setPosition(position);
        }
    }

  // move object
  obj->moveBy(delta);

  // move its label
  CLGraphicalObject* text = getTextForItem(layout, obj);

  if (text != NULL)
    text->moveBy(delta);

  // move species within compartments as well
  CLCompartmentGlyph* lcomp = dynamic_cast<CLCompartmentGlyph*>(obj);

  if (lcomp == NULL)
    return;

  CCompartment*  comp = dynamic_cast<CCompartment*>(lcomp ->getModelObject());

  if (comp == NULL)
    return;

  CDataVectorNS < CMetab > & metabs = comp->getMetabolites();
  CDataVectorNS < CMetab >::const_iterator it = metabs.begin();

  std::set<std::string> reactionKeys;

  while (it != metabs.end())
    {
      moveObject(getMetabGlyphForKey(layout, it), delta, layout);

      CDataVectorNS < CReaction > &  reactions = comp->getObjectDataModel()->getModel()->getReactions();
      CDataVectorNS < CReaction >::const_iterator rit = reactions.begin();

      for (; rit != reactions.end(); ++rit)
        {
          const CReaction* reaction = rit;
          const CChemEq& eqn = reaction->getChemEq();

          const std::set< const CCompartment * >& compartments = eqn.getCompartments();
          std::set< const CCompartment * >::const_iterator cit = compartments.begin();

          for (; cit != compartments.end(); ++cit)
            {
              if ((*cit)->getKey() == comp->getKey())
                reactionKeys.insert(reaction->getKey());
            }
        }

      ++it;
    }

  std::set<std::string>::const_iterator kit = reactionKeys.begin();

  for (; kit != reactionKeys.end(); ++kit)
    {
      moveObject(getReactionGlyphForKey(layout, (*kit)), delta, layout);
    }
}

void CQLayoutScene::updateLock(const QString & key, bool locked)
{
  CKeyFactory * kf = CRootContainer::getKeyFactory();

  if (kf == NULL)
    return;

  CLGraphicalObject * obj = dynamic_cast< CLGraphicalObject * >(kf->get(key.toStdString()));

  if (obj == NULL)
    return;

  obj->setLocked(locked);
}

void CQLayoutScene::updateShow(const QString & key, bool show)
{
  CKeyFactory * kf = CRootContainer::getKeyFactory();

  if (kf == NULL)
    return;

  CLGraphicalObject * obj = dynamic_cast< CLGraphicalObject * >(kf->get(key.toStdString()));

  if (obj == NULL)
    return;

  obj->setShow(show);
}

bool CQLayoutScene::isSideMetabolite(const CMetab * m) const
{
  size_t count = 0;

  // iterate through all reactions of the model
  const auto & reactions = mpLayout->getListOfReactions();
  for (const auto & reaction : reactions)
    {
      // substrates
      const auto & subs = reaction->getChemEq().getSubstrates();
      for (const auto & s : subs)
        {
          if (s.getMetabolite() == m)
            ++count;
        }

      // products
      const auto & prods = reaction->getChemEq().getProducts();
      for (const auto & p : prods)
        {
          if (p.getMetabolite() == m)
            ++count;
        }

      // modifiers
      const auto & mods = reaction->getChemEq().getModifiers();
      for (const auto & mod : mods)
        {
          if (mod.getMetabolite() == m)
            return true;
        }
    }

  return count == 1;
}

void CQLayoutScene::updateSplit(const QString & key, bool split)
{
  CKeyFactory * kf = CRootContainer::getKeyFactory();

  if (kf == NULL)
    return;

  CLGraphicalObject * obj = dynamic_cast< CLGraphicalObject * >(kf->get(key.toStdString()));

  if (obj == NULL)
    return;

  obj->setSplit(split);

  CLMetabGlyph* pMetabGlyph = dynamic_cast< CLMetabGlyph * >(obj);
  if (!pMetabGlyph)
    return;

if (split)
    {
      std::set< const CMetab * > sideMetabs;
      sideMetabs.insert(dynamic_cast< CMetab * >(pMetabGlyph->getModelObject()));

      removeMetab(dynamic_cast< CMetab * >(pMetabGlyph->getModelObject()));

      mpSpringLayout->addSideMetabs(mpLayout, sideMetabs);
    }

// restore lines
CCopasiSpringLayout::Parameters p;
CCopasiSpringLayout l(mpLayout, &p);
l.finalizeState();
emit recreateNeeded();
}

void CQLayoutScene::updatePosition(const QString& key, const QPointF& newPos)
{
  CKeyFactory* kf = CRootContainer::getKeyFactory();

  if (kf == NULL) return;

  CLGraphicalObject* obj = dynamic_cast<CLGraphicalObject*>(kf->get(key.toStdString()));

  if (obj == NULL) return;

  CLPoint delta(newPos.x(), newPos.y());
  moveObject(obj, delta, mpLayout);

  // restore lines
  CCopasiSpringLayout::Parameters p;
  CCopasiSpringLayout l(mpLayout, &p);
  l.finalizeState();

  emit recreateNeeded();
}
void CQLayoutScene::removeMetab(const CMetab * pMetab)
{
  if (!mpLayout || !pMetab)
    return;

  // get all metab glyph keys
  CDataVector< CLMetabGlyph > & metabGlyphs = mpLayout->getListOfMetaboliteGlyphs();
  std::vector< std::string > metabGlyphKeys;

  for (size_t i = 0; i < metabGlyphs.size(); ++i)
    {
      if (metabGlyphs[i].getModelObjectKey() == pMetab->getKey())
        metabGlyphKeys.push_back(metabGlyphs[i].getKey());
    }

  // rm txtglyphs
  CDataVector< CLTextGlyph > & textGlyphs = mpLayout->getListOfTextGlyphs();
  for (size_t i = 0; i < textGlyphs.size();)
    {
      const std::string & gk = textGlyphs[i].getGraphicalObjectKey();
      if (std::find(metabGlyphKeys.begin(), metabGlyphKeys.end(), gk) != metabGlyphKeys.end())
        {
          textGlyphs.remove(i);
        }
      else
        {
          ++i;
        }
    }

  // rm metabrefglyphs
  CDataVector< CLReactionGlyph > & reactions = mpLayout->getListOfReactionGlyphs();
  for (size_t r = 0; r < reactions.size(); ++r)
    {
      CDataVector< CLMetabReferenceGlyph > & refs = reactions[r].getListOfMetabReferenceGlyphs();
      for (size_t i = 0; i < refs.size();)
        {
          if (std::find(metabGlyphKeys.begin(), metabGlyphKeys.end(), refs[i].getMetabGlyphKey()) != metabGlyphKeys.end())
            {
              refs.remove(i);
            }
          else
            {
              ++i;
            }
        }
    }

  // rm metabglyphs
  for (size_t i = 0; i < metabGlyphs.size();)
    {
      if (metabGlyphs[i].getModelObjectKey() == pMetab->getKey())
        {
          metabGlyphs.remove(i);
        }
      else
        {
          ++i;
        }
    }

  // recreate scene
  CCopasiSpringLayout::Parameters p;
  CCopasiSpringLayout l(mpLayout, &p);
  l.finalizeState();
  emit recreateNeeded();
}

bool CQLayoutScene::canSplit(const CLMetabGlyph * pMetabGlyph) const
{
  if (!pMetabGlyph)
    return false;

  const std::string glyphKey = pMetabGlyph->getKey();
  size_t usageCount = 0;

  // Check all reactionglyphs in layout
  const CDataVector< CLReactionGlyph > & reactions = mpLayout->getListOfReactionGlyphs();
  for (size_t r = 0; r < reactions.size(); ++r)
    {
      const CLReactionGlyph & reactionGlyph = reactions[r];
      const CDataVector< CLMetabReferenceGlyph > & refs = reactionGlyph.getListOfMetabReferenceGlyphs();

      for (size_t i = 0; i < refs.size(); ++i)
        {
          if (refs[i].getMetabGlyphKey() == glyphKey)
            {
              usageCount++;
            }
        }
    }

  // If glyph participates in >1 reactions it is splitable
  return (usageCount > 1);
}

bool CQLayoutScene::canMerge() const
{
  const QList< QGraphicsItem * > sel = selectedItems();
  if (sel.size() < 2)
    return false; // merge only if >= 2 items are selected

  std::map< const CMetab *, int > metabCount;

  for (QGraphicsItem * gi : sel)
    {
      // Only consider real styled items
      auto * styled = dynamic_cast< CQStyledGraphicsItem * >(gi);
      if (!styled)
        return false; // foreign type in selection -> cannot merge

      // Get layout key of the glyph
      const QVariant vKey = gi->data(COPASI_LAYOUT_KEY);
      if (!vKey.isValid())
        return false;

      const std::string key = vKey.toString().toStdString();
      CKeyFactory * kf = CRootContainer::getKeyFactory();
      if (!kf)
        return false;

      auto * go = dynamic_cast< CLGraphicalObject * >(kf->get(key));
      auto * metabGlyph = dynamic_cast< CLMetabGlyph * >(go);
      if (!metabGlyph)
        return false; // only metabolites can be merged

      const CMetab * metab = dynamic_cast< const CMetab * >(metabGlyph->getModelObject());
      if (!metab)
        return false;

      metabCount[metab]++;
    }

  // Merge is possible if at least one species has 2 or more glyphs
  for (const auto & [metab, count] : metabCount)
    {
      if (count >= 2)
        return true;
    }

  return false;
}

void CQLayoutScene::mergeSelected()
{
  if (!canMerge())
    return;

  QList< QGraphicsItem * > sel = selectedItems();
  CKeyFactory * kf = CRootContainer::getKeyFactory();
  if (!kf)
    return;

  // Group glyphs by their species
  std::map< const CMetab *, std::vector< const CLMetabGlyph * > > speciesGroups;
  std::map< const CLMetabGlyph *, std::string > glyphKeys;

  for (QGraphicsItem * gi : sel)
    {
      const QVariant vKey = gi->data(COPASI_LAYOUT_KEY);
      if (!vKey.isValid())
        continue;

      const std::string key = vKey.toString().toStdString();
      auto * go = dynamic_cast< CLGraphicalObject * >(kf->get(key));
      auto * metabGlyph = dynamic_cast< const CLMetabGlyph * >(go);
      if (!metabGlyph)
        continue;

      const CMetab * metab = dynamic_cast< const CMetab * >(metabGlyph->getModelObject());
      if (!metab)
        continue;

      speciesGroups[metab].push_back(metabGlyph);
      glyphKeys[metabGlyph] = key;
    }

  // Merge each group with at least 2 glyphs
  for (auto & [metab, glyphs] : speciesGroups)
    {
      if (glyphs.size() < 2)
        continue; // skip groups that cannot be merged

      const std::string targetKey = glyphKeys[glyphs.front()];

      // Collect keys of glyphs to remove
      std::set< std::string > keysToRemove;
      for (size_t i = 1; i < glyphs.size(); ++i)
        keysToRemove.insert(glyphKeys[glyphs[i]]);

      // Redirect references in reaction glyphs
      CDataVector< CLReactionGlyph > & reactions = mpLayout->getListOfReactionGlyphs();
      for (size_t r = 0; r < reactions.size(); ++r)
        {
          CDataVector< CLMetabReferenceGlyph > & refs = reactions[r].getListOfMetabReferenceGlyphs();
          for (size_t i = 0; i < refs.size(); ++i)
            {
              const std::string & k = refs[i].getMetabGlyphKey();
              if (keysToRemove.count(k))
                refs[i].setMetabGlyphKey(targetKey);
            }
        }

      // Remove text glyphs belonging to merged glyphs
      CDataVector< CLTextGlyph > & textGlyphs = mpLayout->getListOfTextGlyphs();
      for (size_t i = 0; i < textGlyphs.size();)
        {
          if (keysToRemove.count(textGlyphs[i].getGraphicalObjectKey()))
            textGlyphs.remove(i);
          else
            ++i;
        }

      // Remove merged metabolite glyphs
      CDataVector< CLMetabGlyph > & metabGlyphs = mpLayout->getListOfMetaboliteGlyphs();
      for (size_t i = 0; i < metabGlyphs.size();)
        {
          if (keysToRemove.count(metabGlyphs[i].getKey()))
            metabGlyphs.remove(i);
          else
            ++i;
        }

      // Reset split flag on target glyph and apply a small random offset
      if (auto * goTarget = dynamic_cast< CLGraphicalObject * >(kf->get(targetKey)))
        {
          goTarget->setSplit(false);
          CLPoint pos = goTarget->getPosition();
          double dx = ((rand() % 5) - 2); // shift between -2 and +2
          double dy = ((rand() % 5) - 2);
          pos.setX(pos.getX() + dx);
          pos.setY(pos.getY() + dy);
          goTarget->setPosition(pos);
        }
    }

  // Re-run spring layout and trigger scene update
  CCopasiSpringLayout::Parameters p;
  CCopasiSpringLayout l(mpLayout, &p);
  l.finalizeState();

  emit recreateNeeded();
}