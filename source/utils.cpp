#include "utils.hpp"
#include "level.hpp"

#include <QLineF>
#include <QFile>
#include <QMessageBox>
#include <QtSvg/QSvgRenderer>

#include <cmath>
#include <cstdlib>

const char *kGameModeName[NumGameModes] = { "no mode",
                                            "main menu",
                                            "built-in level chooser",
                                            "user level chooser",
                                            "playing level",
                                            "paused",
                                            "level editor" };

qreal distBetweenPoints(QPointF point1, QPointF point2)
{
  return fabs(QLineF(point1, point2).length());
}

unsigned long int max(unsigned long int a, unsigned long int b)
{
  if(a >= b)
    return a;

  return b;
}

unsigned long int min(unsigned long int a, unsigned long int b)
{
  if(a <= b)
    return a;

  return b;
}

qreal max(qreal a, qreal b)
{
  if(a >= b)
    return a;

  return b;
}

qreal min(qreal a, qreal b)
{
  if(a <= b)
    return a;

  return b;
}

bool checkFileExists(const QString filename, bool displayWarning)
{
	if(!QFile(filename).exists())
	{
	  if(displayWarning)
	  {
      QMessageBox msgBox;
      QString text = QString("File %1 doesn't exist!").arg(filename);
      msgBox.setText(text);
      msgBox.exec();
	  }
		return false;
	}

	return true;
}

long unsigned int getRandom(long unsigned int minimum, long unsigned int maximum)
{
  if(minimum > maximum)
  {
    qDebug("invalid parameters to getRandom()!");
    return 0;
  }

  return (qrand() % (maximum - minimum + 1)) + minimum;
}

qreal getRandomF(qreal minimum, qreal maximum)
{
  if(minimum > maximum)
  {
    qDebug("invalid parameters to getRandomF()!");
    return 0;
  }

  return ((qreal)qrand()) / (qreal)(RAND_MAX) * (maximum - minimum) + minimum;
}

QPixmap* loadPngImage(const QString filename)
{
  QString tmp_name = QString("%1%2").arg(FILE_PREFIX).arg(filename);

  if(!checkFileExists(tmp_name, true))
    exit(EXIT_FAILURE);

  QPixmap* p_pixmap = new QPixmap(tmp_name);
  return p_pixmap;
}

QGraphicsSvgItem* loadSvgImage(const QString filename)
{
  QString tmp_name = QString("%1%2").arg(FILE_PREFIX).arg(filename);
  if(!checkFileExists(tmp_name, true))
    exit(EXIT_FAILURE);

  QGraphicsSvgItem* p_svgImg = new QGraphicsSvgItem(tmp_name);
  return p_svgImg;
}

void loadSvgImage(QSvgRenderer **ppRenderer, const QString filename)
{
  QString tmp_name = QString("%1%2").arg(FILE_PREFIX).arg(filename);

  if(!checkFileExists(tmp_name, true))
    exit(EXIT_FAILURE);

  *ppRenderer = new QSvgRenderer(tmp_name);

  if(*ppRenderer == NULL)
    exit(EXIT_FAILURE);
}

#ifndef QT_NO_SOUND
QSound* loadSound(const QString filename)
{
  qDebug("Loading sound from '%s'...", filename.toAscii().data());

  QSound *p_sound = new QSound(filename);
  return p_sound;
}
#endif // !QT_NO_SOUND

void setGlobals(bool first)
{
  // DIMENSIONS

  //!!! check for available point device (set to false for e.g. S60v3)
  if(1/*!!! mouse_available*/)
  {
    gUseMouseEvents = true;

    gScreenShrinkFactor = gWindowHeight / 480.0;

    gGuiSpacer = kDefaultGuiSpacer * gScreenShrinkFactor;

    if(gVerticalBars)
      gInfoBarLength = gWindowHeight / 2.5 * gScreenShrinkFactor;
    else
      gInfoBarLength = gWindowWidth / 2.2 * gScreenShrinkFactor;
    gInfoBarWidth = 12.0 * gScreenShrinkFactor;

    gInfoFontSize = kInfoBarToFontFactor * gInfoBarWidth;
    gEffectiveInfoFontSize = gInfoFontSize * kEffectiveFontSizeFactor;

    // Info area should be high enough to show 3 lines of text + space between.
    gInfoAreaSize = (6.0 * gGuiSpacer) + (3.0 * gEffectiveInfoFontSize);
  }
  else
  {
    gUseMouseEvents = false;

    gScreenShrinkFactor = gWindowHeight / 320.0;

    gGuiSpacer = kDefaultGuiSpacer * gScreenShrinkFactor;

    if(gVerticalBars)
    {
      gInfoBarLength = 12.0 * gScreenShrinkFactor;
      gInfoBarWidth = gWindowHeight / 4.0 * gScreenShrinkFactor;
    }
    else
    {
      gInfoBarLength = gWindowWidth / 4.0 * gScreenShrinkFactor;
      gInfoBarWidth = 12.0 * gScreenShrinkFactor;
    }

    gInfoFontSize = kInfoBarToFontFactor * gInfoBarWidth;
    gEffectiveInfoFontSize = gInfoFontSize + 7.0;

    // Info area should be high enough to show 2 lines of text + space between.
    gInfoAreaSize = (2.5 * gGuiSpacer) + (2.0 * gEffectiveInfoFontSize);
  }

  gLevelZoomFactor *= gScreenShrinkFactor;

  if(gVerticalBars)
    gBarToLabelOffsetX = 4.0;
  else
    gBarToLabelOffsetX = 0.0;

  gBarToLabelOffsetY = ((gEffectiveInfoFontSize - gInfoBarWidth) / 2.0) + 5.0;
  gGuiSpacer = 7.5 * gScreenShrinkFactor;

  gDistBarkPowerLabelToBar = kDefaultDistBarkPowerLabelToBar;
  gDistTimeLabelToBar      = kDefaultDistTimeLabelToBar;

  gSceneToViewOffset = QPointF((kSceneWidth - gWindowWidth) / 2,
                               (kSceneHeight - gWindowHeight) / 2);

  gDogMinDistToBorder = 140.0 * gScreenShrinkFactor;


  // OTHER GLOBALS
  gNumLevelElmtStyles[UnknownLevelElement] = 0;
  gNumLevelElmtStyles[DecoLevelElement]    = kNumDecoStyles;
  gNumLevelElmtStyles[ExitLevelElement]    = kNumExitStyles;
  gNumLevelElmtStyles[FenceLevelElement]   = kNumFenceStyles;
  gNumLevelElmtStyles[WaterLevelElement]   = kNumWaterStyles;
  gNumLevelElmtStyles[TreeLevelElement]    = kNumTreeStyles;
  gNumLevelElmtStyles[BushLevelElement]    = kNumBushStyles;
}

void showPausedScreen(bool show)
{
  gInactiveBackground->setVisible(show);
  gStopButtonItem->setVisible(show);
  gRetryButtonItem->setVisible(show);
  gPlayButtonItem->setVisible(show);
}

const char* getModeName(GameMode mode)
{
  return kGameModeName[mode];
}

GameMode curGameMode()
{
  if(gModeList.empty())
  {
    qDebug("no current mode!");
    return NoGameMode;
  }

  return gModeList.back();
}

GameMode prevGameMode()
{
  if(gModeList.size() <= 1)
  {
    qDebug("no previous mode!");
    return NoGameMode;
  }

  return gModeList.at(gModeList.size()-2);
}

GameMode prePrevGameMode()
{
  if(gModeList.size() <= 2)
  {
    qDebug("no pre-previous mode!");
    return NoGameMode;
  }

  return gModeList.at(gModeList.size()-3);
}

void addGameMode(GameMode mode)
{
  if(mode == curGameMode())
    qDebug("current mode already set to '%s'", getModeName(mode));
  else
    gModeList.append(mode);
}

void setGameMode(GameMode mode)
{
  gModeList.clear();
  gModeList.append(mode);
}

void goToMainMenuMode()
{
  // there's no "go back button" (or anything similar) here, so set current and previous mode
  setGameMode(MainMenuMode);
  ((Level*)gLevel)->menu();
}

void goToGameMode(GameMode mode)
{
  switch(mode)
  {
    case MainMenuMode:
      goToMainMenuMode();
      break;
    case BuiltInChooserMode:
      addGameMode(mode);
      gNextMode = PlayMode;
      if(!((Level*)gLevel)->showChooserDialog(BuiltInSource))
        goToMainMenuMode();
      break;
    case UserChooserMode:
      addGameMode(mode);
      gNextMode = PlayMode;
      if(!((Level*)gLevel)->showChooserDialog(UserSource))
        goToMainMenuMode();
      break;
    case EditChooserMode:
      addGameMode(mode);
      gNextMode = LevelEditMode;
      if(!((Level*)gLevel)->showChooserDialog(UserSource))
        goToGameMode(gNextMode);
      break;
    case PlayMode:
      if(curGameMode() == BuiltInChooserMode)
      {
        addGameMode(mode);
        if(((Level*)gLevel)->loadLevel(BuiltInSource))
        {
          if(((Level*)gLevel)->start())
            return;
        }
      }
      else if(curGameMode() == UserChooserMode)
      {
        addGameMode(mode);
        if(((Level*)gLevel)->loadLevel(UserSource))
        {
          if(((Level*)gLevel)->start())
            return;
        }
      }
      else if(curGameMode() == LevelEditMode)
      {
        addGameMode(mode);
        if(((Level*)gLevel)->start())
          return;
      }
      else if(curGameMode() == PlayMode)
      {
        if(((Level*)gLevel)->start())
          return;
      }
      else
      {
        qDebug("invalid mode %d to go to play mode!", mode);
      }

      goToMainMenuMode();
      break;
    case PausedMode:
      qDebug("please call pauseGame() directly!");
      break;
    case LevelEditMode:
      // the previous mode is not needed here because all mode changes in editor are determined by
      // certain actions (e.g. button press)
      setGameMode(mode);
      ((Level*)gLevel)->editor();
      break;
    default:
      qDebug("goToGameMode(): invalid mode %d!", mode);
      goToMainMenuMode();
      break;
  }
}

void restorePrevGameMode(bool undoButton, bool stopButton)
{
  GameMode currentMode = curGameMode();
  GameMode previousMode = prevGameMode();
  gModeList.pop_back();

  switch(currentMode)
  {
    case MainMenuMode:
      exit(EXIT_SUCCESS);
      break;

    case BuiltInChooserMode:
      goToMainMenuMode();
      break;

    case UserChooserMode:
      if(previousMode == MainMenuMode || previousMode == LevelEditMode)
      {
        goToGameMode(previousMode);
      }
      else
      {
        qDebug("invalid game mode previous to user level choice");
        goToMainMenuMode();
      }
      break;

    case EditChooserMode:
      goToGameMode(previousMode);
      break;

    case PlayMode:
      if(previousMode == BuiltInChooserMode || previousMode == UserChooserMode)
      {
        goToGameMode(previousMode);
      }
      else if(previousMode == LevelEditMode)
      {
        ((Level*)gLevel)->resetCurrentLevel();
        ((Level*)gLevel)->editor();
      }
      else
      {
        qDebug("invalid mode %d before play mode!", currentMode);
        goToMainMenuMode();
      }
      break;

    case PausedMode:
      if(undoButton || stopButton)
      {
        showPausedScreen(false);
        gModeList.pop_back();
        goToGameMode(curGameMode());
      }
      else
      {
        qDebug("may only call restorePrevGameMode() from paused mode for undo/stop button!");
        goToMainMenuMode();
      }
      break;

    case LevelEditMode:
      goToMainMenuMode();
      break;

    default:
      qDebug("restorePrevGameMode(): invalid mode %d!", currentMode);
      goToMainMenuMode();
      break;
  }
}
