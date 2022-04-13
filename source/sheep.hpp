#ifndef SHEEP_HPP
#define SHEEP_HPP

#include "character.hpp"
#include "exit.hpp"
#include "level_element.hpp"

class Sheep : public Character
{
public:
  Sheep(qreal sx, qreal sy);
  void init();

  void init(void *pLevel);

	void refreshSpeed();
	void turnAround();
	void intelligentTurnAround();
  void goToExit();

#ifdef DEBUG
  void drawColRects();
#endif

	// set if sheep has been "saved"
	bool rescued;

	// set if sheep is blocked (collisions on all sides)
	bool blocked;
	
	bool sleeping;

protected:
  // QGraphicsItem method
  void advance(int step);

private:
#ifdef SHEEP_TO_SHEEP_COLLISION
  QList<Sheep *> *p_sheep_list;
#endif

  void *p_level;

  long unsigned int baaFrame;
};

#endif // SHEEP_HPP
