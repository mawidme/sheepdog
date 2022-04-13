#ifndef LEVEL_CHOOSE_ITEMS_HPP_
#define LEVEL_CHOOSE_ITEMS_HPP_

#include "level_types.hpp"

#include <QGraphicsItem>
#include <QtSvg/QGraphicsSvgItem>

class LevelChooseItem : public QGraphicsItem
{
public:
  LevelChooseItem(const long int number, qreal width);
  bool contains(const QPointF &point) const;
  long int number();

  // virtual QGraphicsItem method.
  QRectF boundingRect() const;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {};

private:
  QGraphicsTextItem m_text;
  QGraphicsRectItem m_frame;

  long int m_number;
};

class LevelChooseItems : public QGraphicsItem
{
public:
  LevelChooseItems();
  ~LevelChooseItems();
  void clearList();

  QRectF boundingRect() const;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

  bool init(const LevelSource source, bool rescanLevels);

  int checkClick(QPointF scenePoint);

private:
  LevelSource m_source;

  QGraphicsRectItem m_frame;
  QList<LevelChooseItem*> m_levelItemList;
  QGraphicsSvgItem    *p_leaveItem;

  qreal m_itemWidth;

  bool m_initialized;
};

#endif /* LEVEL_CHOOSE_ITEMS_HPP_ */
