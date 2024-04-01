#ifndef LEVEL_ELEMENT_HPP
#define LEVEL_ELEMENT_HPP

#include "globals.hpp"

#include "game_object.hpp"

#include <QtWidgets/QGraphicsItem>

class LevelElement : public GameObject
{
public:
  LevelElement();
  ~LevelElement();

  QSizeF loadElmtGraphics(long unsigned int sx, long unsigned int sy);

  void initColRect(QRectF rect);
  void initColRect(qreal x, qreal y, qreal width, qreal height);

  LevelElemType getElemType();
  QRectF* getCollisionRect();

  // QGraphicsItem methods
  virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget);
  QRectF boundingRect() const;

  void moveByDelta(qreal dx, qreal dy);
  void moveByDeltaPoint(QPointF d);

  long unsigned int m_layer;

  long unsigned int m_imgStyle;

  // data for different image types
  QGraphicsPixmapItem *p_pngImg;
  QList<QGraphicsPixmapItem *> m_pngImgList; //!!! handling of this type is incomplete!
  QGraphicsSvgItem *p_svgImg;

#ifdef SHOW_COL_RECTS
  // item to draw collision rectangles
  QGraphicsRectItem m_colRectItem;
#endif

protected:
  // QGraphicsItem method
  void advance(int step);

  LevelElemType m_elemType;
  QRectF *p_colRect;

  QRectF m_boundingRect;
};

#endif // LEVEL_ELEMENT_HPP
