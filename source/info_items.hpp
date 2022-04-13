#ifndef INFOS_ITEM_HPP_
#define INFOS_ITEM_HPP_

#include <QGraphicsItem>

class InfoItems : public QGraphicsItem
{
public:
  InfoItems();
  ~InfoItems();

  QRectF boundingRect() const;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

  void init();
  void setTextAndColors();
  void setGeometry();

  void updateBarkPowerBar();
  void updateTime(long unsigned int timeLeft, long unsigned int totalTime, long unsigned int bonusTime = 0);
  void updateScore();

private:
  QGraphicsRectItem   m_infosBackground;

  QGraphicsTextItem   m_barkPowerLabel;
  QGraphicsRectItem   m_barkPowerBar;

  QGraphicsTextItem   m_timeLabel;
  QGraphicsRectItem   m_timeBar;
  QGraphicsRectItem   m_bonusTimeBar;

  QGraphicsTextItem   m_scoreLabel;
  QGraphicsTextItem   m_scoreCounter;

  bool m_initialized;
};

#endif /* INFOS_ITEM_HPP_ */
