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

#include <copasi/qlayout/CQCopasiGraphicsItem.h>
#include <copasi/layout/CLStyle.h>
#include <copasi/layout/CLRenderResolver.h>

CQCopasiGraphicsItem::~CQCopasiGraphicsItem()
{
}

CQCopasiGraphicsItem::CQCopasiGraphicsItem(const CLRenderResolver* resolver, const CLStyle* style)
  : mpStyle(style)
  , mpResolver(resolver)
  , mIsValid(true)
  , mLocked(false)
  , mSplit(false)
{
}

void CQCopasiGraphicsItem::setResolver(const CLRenderResolver* resolver)
{
  mpResolver = resolver;
}

const CLRenderResolver* CQCopasiGraphicsItem::getResolver() const
{
  return mpResolver;
}

void CQCopasiGraphicsItem::setStyle(const CLStyle* style)
{
  mpStyle = style;
}

const CLStyle* CQCopasiGraphicsItem::getStyle() const
{
  return mpStyle;
}

bool CQCopasiGraphicsItem::isValid() const
{
  return mIsValid;
}

bool CQCopasiGraphicsItem::isLocked() const
{
  return mLocked;
}

bool CQCopasiGraphicsItem::isShow() const
{
  return mShow;
}

bool CQCopasiGraphicsItem::isSplit() const
{
  return mSplit;
}

void CQCopasiGraphicsItem::setLocked(bool locked)
{
  mLocked = locked;
}

void CQCopasiGraphicsItem::setShow(bool show)
{
  mShow = show;
}

void CQCopasiGraphicsItem::setSplit(bool split)
{
  mSplit = split;
}
