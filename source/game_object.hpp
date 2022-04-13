#ifndef GAME_OBJECT_HPP_
#define GAME_OBJECT_HPP_

#include "types.hpp"

#include <QGraphicsItem>
#include <QPointF>


class GameObject : public QGraphicsItem
{
public:
  GameObject();
  ~GameObject();

  // QGraphicsItem methods
  virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget);
  virtual QRectF boundingRect() const;

  QPointF getTopLeftPos();
  QPointF getBottomRightPos();

  bool added();
  void setAdded();

protected:
  // QGraphicsItem method
  virtual void advance(int step);

  QString getImageSuffix();

  ImageType m_imgType;
  QString   m_imgFileName; // file name without prefix and suffix

  QPointF m_topLeftPos;
  QPointF m_bottomRightPos;

  bool m_imgInit;

private:
  bool m_addedToLevel;
};

#endif /* GAME_OBJECT_HPP_ */
