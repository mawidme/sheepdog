#ifndef DOG_HPP
#define DOG_HPP

#include "globals.hpp"
#include "character.hpp"
#include "sheep.hpp"

class Dog : public Character
{
public:
  Dog(qreal sx, qreal sy);
  void init();

  void zeroSpeedX();
  void zeroSpeedY();
  bool bark();

#ifdef DEBUG
  void drawColRects();
#endif

  qreal m_barkPower;

  QPointF m_curTargetPoint;

protected:
  // QGraphicsItem method
  void advance(int step);

private:
	void refreshColRect();
};

#endif // DOG_HPP
