#ifndef UTILS_HPP
#define UTILS_HPP

#include "types.hpp"

#include "globals.hpp"

#include <QPointF>
#include <QString>

qreal distBetweenPoints(QPointF point1, QPointF point2);
unsigned long int max(unsigned long int a, unsigned long int b);
unsigned long int min(unsigned long int a, unsigned long int b);

qreal max(qreal a, qreal b);
qreal min(qreal a, qreal b);

bool checkFileExists(const QString filename, bool displayWarning = false);

long unsigned int getRandom(long unsigned int minimum, long unsigned int maximum);
qreal getRandomF(qreal minimum, qreal maximum);

QPixmap* loadPngImage(const QString filename);
QGraphicsSvgItem* loadSvgImage(const QString filename);
void loadSvgImage(QSvgRenderer **ppRenderer, const QString filename);

#ifndef QT_NO_SOUND
QSound* loadSound(const QString filename);
#endif

void setGlobals(bool first);

void showPausedScreen(bool show);

const char* getModeName(GameMode mode);
GameMode curGameMode();
GameMode prevGameMode();
GameMode prePrevGameMode();

// sets game mode deleting mode history
void addGameMode(GameMode mode);
void setGameMode(GameMode mode);

void goToMainMenuMode();
void goToGameMode(GameMode mode);
void restorePrevGameMode(bool undoButton, bool stopButton = false);

#endif // UTILS_HPP
