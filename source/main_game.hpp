#ifndef MAIN_GAME_HPP
#define MAIN_GAME_HPP

#include "level.hpp"

#include "mygraphicsscene.hpp"

#include <QGraphicsView>
#include <QSound>
#include <QtSvg/QSvgRenderer>

class MainGame
{
public:
  MainGame();
  ~MainGame();

	bool init();

  void initColorsAndStyles();

	bool loadGraphics();

	bool loadSounds();

  void loop();
	void startGame();
	void gotoStart();
	void updateTimeBarAndScore(long unsigned int time);
	void createInfoText();
	void removeInfoText();
	void createBarkPowerBar();
  void refreshBarkPowerBar(long int power);
	void timeIsUp();
  void stop();

	void levelDone();

  Level m_level;

	// info area offset (bark power, time, ...)
	long int xInfoOffset;
	long int yInfoOffset;

#ifndef QT_NO_SOUND
	QSound *music;
#endif
};

#endif // MAIN_GAME_HPP
