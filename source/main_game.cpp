#include "main_game.hpp"

#include "utils.hpp"

#include <QtGui/QKeyEvent>
#include <QtCore/QTime>
#include <QtCore/qmath.h>

MainGame::MainGame()
{
#ifndef QT_NO_SOUND
	music = NULL;
#endif
}

MainGame::~MainGame()
{
#ifndef QT_NO_SOUND
	// LEVEL
  if(levelSounds != NULL)
  {
    for(int i=0; i<kNumLevelSounds; i++)
      if(levelSounds[i] != NULL)
        delete levelSounds[i];
    delete levelSounds;
    levelSounds = NULL;
  }
  if(gSheepRescuedSound != NULL)
  {
    for(int i=0; i<kNumSheepRescuedSounds; i++)
      if(gSheepRescuedSound[i] != NULL)
        delete gSheepRescuedSound[i];
    delete gSheepRescuedSound;
    gSheepRescuedSound = NULL;
  }
  if(gLevelDoneSound != NULL)
  {
    delete gLevelDoneSound;
    gLevelDoneSound = NULL;
  }
  if(gTryAgainSound != NULL)
  {
    delete gTryAgainSound;
    gTryAgainSound = NULL;
  }
  if(gGameOverSound != NULL)
  {
    delete gGameOverSound;
    gGameOverSound = NULL;
  }

  // SHEEP
  if(gSheepSound != NULL)
  {
    for(int i=0; i<kNumSheepSounds; i++)
      if(gSheepSound[i] != NULL)
        delete gSheepSound[i];

    delete gSheepSound;
    gSheepSound = NULL;
  }

  // DOG
  if(gBarkSounds != NULL)
  {
    for(int i=0; i<kNumBarkSounds; i++)
      if(gBarkSounds[i] != NULL)
        delete gBarkSounds[i];

    delete gBarkSounds;
    gBarkSounds = NULL;
  }
  if(gBarkPowerCriticalSound)
  {
    delete gBarkPowerCriticalSound;
    gBarkPowerCriticalSound = NULL;
  }
  if(gBarkPowerFullSound)
  {
    delete gBarkPowerFullSound;
    gBarkPowerFullSound = NULL;
  }
  if(gNoBarkSound)
  {
    delete gNoBarkSound;
    gNoBarkSound = NULL;
  }
#endif // QT_SOUND
}

bool MainGame::init()
{
	if(!loadGraphics())
		return false;

  if(!loadSounds())
    return false;

  initColorsAndStyles();

  gLevel = (void*)&m_level;
  m_level.init();

  goToMainMenuMode();

#ifdef DEBUG
  trace("initialized");
#endif /* DEBUG */

  return true;
}

bool MainGame::loadGraphics()
{
  QString filename;
  QString styleNumber;


  // GUI / INFOS
  if(gBarkButtonItem == NULL)
    if((gBarkButtonItem = loadSvgImage("img/barkButton.svg")) == NULL)
      return false;

  if(gPauseButtonItem == NULL)
    if((gPauseButtonItem = loadSvgImage("img/pauseButton.svg")) == NULL)
      return false;

  if(gStopButtonItem == NULL)
    if((gStopButtonItem = loadSvgImage("img/stopButton.svg")) == NULL)
      return false;

  if(gRetryButtonItem == NULL)
    if((gRetryButtonItem = loadSvgImage("img/retryButton.svg")) == NULL)
      return false;

  if(gPlayButtonItem == NULL)
    if((gPlayButtonItem = loadSvgImage("img/playButton.svg")) == NULL)
      return false;

	return true;
}

bool MainGame::loadSounds()
{
#ifndef QT_NO_SOUND
  // LEVEL
  if(levelSounds == NULL)
  {
    levelSounds = new QSound*[kNumLevelSounds];
    for(int i=0; i<kNumLevelSounds; i++)
    {
      QString filename = QString("sfx/level%1.wav").arg(i+1);
      if((levelSounds[i] = loadSound(filename)) == NULL)
        return false;
    }
  }

  if(!gLevelDoneSound && (gLevelDoneSound = loadSound("sfx/levelDone.wav")) == NULL)
    return false;
  if(!gTryAgainSound && (gTryAgainSound = loadSound("sfx/tryAgain.wav")) == NULL)
    return false;
  if(!gGameOverSound && (gGameOverSound = loadSound("sfx/gameOver.wav")) == NULL)
    return false;

  // SHEEP
  if(gSheepSound == NULL)
  {
    gSheepSound = new QSound*[kNumSheepSounds];
    for(int i=0; i<kNumSheepSounds; i++)
    {
      QString filename = QString("sfx/sheep%1.wav").arg(i+1);
      if((gSheepSound[i] = loadSound(filename)) == NULL)
        return false;
    }
  }
  if(gSheepRescuedSound == NULL)
  {
    gSheepRescuedSound = new QSound*[kNumSheepRescuedSounds];
    for(int i=0; i<kNumSheepRescuedSounds; i++)
    {
      QString filename = QString("sfx/rescue%1.wav").arg(i+1);
      if((gSheepRescuedSound[i] = loadSound(filename)) == NULL)
        return false;
    }
  }


  // DOG
  if((gBarkPowerCriticalSound = loadSound("sfx/barkPowerCritical.wav")) == NULL)
    return false;
  if((gBarkPowerFullSound = loadSound("sfx/barkPowerFull.wav")) == NULL)
    return false;
  if((gNoBarkSound = loadSound("sfx/noBark.wav")) == NULL)
    return false;
  if(gBarkSounds == NULL)
  {
    gBarkSounds = new QSound*[kNumBarkSounds];
    for(int i=0; i<kNumBarkSounds; i++)
    {
      QString filename = QString("sfx/bark%1.wav").arg(i+1);
      if((gBarkSounds[i] = loadSound(filename)) == NULL)
        return false;
    }
  }
#endif // !QT_NO_SOUND

  return true;
}

void MainGame::initColorsAndStyles()
{
  // COLORS
  QLinearGradient gradient(0, 0, gInfoBarLength, 0);
  gradient.setColorAt(0, QColor(150, 0, 0, kInfoAlpha));
  gradient.setColorAt(1, QColor(0, 150, 0, kInfoAlpha));
  gTimeBarBrush = QBrush(gradient);

  // PEN WIDTHS
  gFenceSlatPen.setWidthF(1.0);
  gFencePolePen.setWidthF(1.0);
  gBarkWavePen.setWidthF(kBarkWaveWidth);
  gInfoBackgroundPen.setWidthF(0.0);

  // FONTS
  gInfoFont = QFont();
  gInfoFont.setBold(true);
  gInfoFont.setPixelSize(qFloor(gInfoFontSize));

}
