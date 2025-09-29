// Copyright (C) 2017 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and University of
// of Connecticut School of Medicine.
// All rights reserved.

// Copyright (C) 2013 - 2016 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

#ifndef QCOPASI_GRAPHICS_ITEM
#define QCOPASI_GRAPHICS_ITEM

#define COPASI_OBJECT_CN 1
#define COPASI_LAYOUT_KEY 2

class CLStyle;
class CLRenderResolver;
class CQCopasiGraphicsItem
{
public:
  CQCopasiGraphicsItem(const CLRenderResolver* resolver, const CLStyle* style);
  virtual ~CQCopasiGraphicsItem();
  void setResolver(const CLRenderResolver* resolver);
  const CLRenderResolver* getResolver() const;
  void setStyle(const CLStyle* style);
  const CLStyle* getStyle() const;
  bool isValid() const;

  /**
   * @brief Returns whether the item is locked.
   * A locked item cannot be moved or edited
   * @return True if the item is locked, false otherwise.
   */
  bool isLocked() const;

  /**
   * @brief Returns whether the bezier control points of connection are visible.
   * @return True if the control points are shown, false otherwise.
   */
  bool isShow() const;

  /**
   * @brief Returns whether the metabolite is marked to be split.
   * @return True if the metabolite is supposed to be split, false otherwise.
   */
  bool isSplit() const;

  /**
   * @brief Sets the lock state of the item.
   * @param locked True to lock the item (non-movable), false to unlock it.
   */
  void setLocked(bool locked);

  /**
   * @brief Sets whether the bezier control points of connection should be shown.
   * @param show True to show control points, false to hide them.
   */
  virtual void setShow(bool show);

  /**
   * @brief Sets whether the metabolite is marked to be split.
   * @param split True to mark as to be split, false otherwise.
   */
  void setSplit(bool split);

protected:
  const CLStyle* mpStyle;
  const CLRenderResolver* mpResolver;
  bool mIsValid;
  bool mLocked; ///< Lock state of the item (prevents moving)
  bool mShow; ///< Whether bezier control points are visible
  bool mSplit; ///< Whether the metabolite is marked as to be split
};

#endif
