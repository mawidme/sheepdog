#include "bonus.hpp"

#include "utils.hpp"
#include "globals.hpp"
#include "global_lists.hpp"
#include "level.hpp"


static const unsigned int kNumSecondsToShowModeImage = 2;
static const qreal kMinShowModeImageScale = 0.1;
static const qreal kMaxShowModeImageScale = 1.0;

static const qreal kPowerBarkReload = 1.0;
static const qreal kPowerBarkDist = 400.0;
static const qreal kPowerBarkIntensityConstant = 500.0;
static const qreal kPowerBarkSlowDown = 0.12; //!!! replace with time factor

// probability that shown box contains a surprise = 1 : kSurpriseRatio
static const long unsigned int kSurpriseRatio = 3;

static const char *kBonusModeText[NumBonusModes] = { "drunk mode",
                                                     "sheep sleep mode",
                                                     "sheep on speed mode",
                                                     "time bonus mode",
                                                     "power bark mode" };

static const char *kModeImgFilenames[NumBonusModes] = { "img/bonus/drunkMode",
                                                        "img/bonus/sheepSleepMode",
                                                        "img/bonus/sheepOnSpeedMode",
                                                        "img/bonus/timeBonusMode",
                                                        "img/bonus/powerBarkMode" };

static const char *kBoxModeImgFilenames[NumBonusModes] = { "img/bonus/drunkBox",
                                                           "img/bonus/sheepSleepBox",
                                                           "img/bonus/sheepOnSpeedBox",
                                                           "img/bonus/timeBonusBox",
                                                           "img/bonus/powerBarkBox" };

static const char *kBoxImageFilename = "img/bonus/box";
static const char *kSurpriseImageFilename = "img/bonus/surprise";

static const int kBonusModeDuration[NumBonusModes] = {10, 6, 10, 10, 15};

Bonus::Bonus() :  p_level(NULL), p_view(NULL), p_scene(NULL), m_imgInit(false),
                  m_curMode(NoBonusMode), m_surprise(false), m_boxVisible(false), m_bonusPickedUp(false), m_modeVisible(false)
{
  p_boxImgItem = NULL;
  p_boxPngImg = NULL;
  p_boxPngImgItem = NULL;
  p_boxSvgImgItem = NULL;

  p_surpriseImgItem = NULL;
  p_surprisePngImg = NULL;
  p_surprisePngImgItem = NULL;
  p_surpriseSvgImgItem = NULL;

  for(int i=0; i<NumBonusModes; i++)
  {
    p_boxModeImgItem[i] = NULL;
    p_boxModePngImg[i] = NULL;
    p_boxModePngImgItem[i] = NULL;
    p_boxModeSvgImgItem[i] = NULL;

    p_modeImgItem[i] = NULL;
    p_modePngImg[i] = NULL;
    p_modePngImgItem[i] = NULL;
    p_modeSvgImgItem[i] = NULL;
  }

  m_curModeActivateFrame = 0;
	m_curModeDeactivateFrame = 0;
	
	m_curModeImageHideFrame = 0;

	m_bonusPickedUp = false;
}

Bonus::~Bonus()
{
  delImg();
}

bool Bonus::init(void *pLevel, MyGraphicsView *pView, MyGraphicsScene *pScene)
{
  p_level = pLevel;
  p_view = pView;
  p_scene = pScene;

  setNextBonusFrames();

  initImg();

  return true;
}

void Bonus::initImg()
{
  // if initialized before, delete all graphics first (e.g. on quality switch)
  if(m_imgInit)
    delImg();

  if(gGraphicQuality >= HighQuality)
    m_imgType = SvgImgType;
  else
    m_imgType = PngImgType;

  if(m_imgType == PngImgType)
  {
    p_boxPngImg = loadPngImage(QString("%1.png").arg(kBoxImageFilename));
    p_boxPngImgItem = new QGraphicsPixmapItem(*p_boxPngImg);
    p_boxImgItem = p_boxPngImgItem;

    p_surprisePngImg = loadPngImage(QString("%1.png").arg(kSurpriseImageFilename));
    p_surprisePngImgItem = new QGraphicsPixmapItem(*p_surprisePngImg);
    p_surpriseImgItem = p_surprisePngImgItem;

    for(int i=0; i<NumBonusModes; i++)
    {
      p_modePngImg[i] = loadPngImage(QString("%1.png").arg(kModeImgFilenames[i]));
      p_modePngImgItem[i] = new QGraphicsPixmapItem(*p_modePngImg[i]);
      p_modeImgItem[i] = p_modePngImgItem[i];
    }

    for(int i=0; i<NumBonusModes; i++)
    {
      p_boxModePngImg[i] = loadPngImage(QString("%1.png").arg(kBoxModeImgFilenames[i]));
      p_boxModePngImgItem[i] = new QGraphicsPixmapItem(*p_boxModePngImg[i]);
      p_boxModeImgItem[i] = p_boxModePngImgItem[i];
    }
  }
  else
  {
    p_boxSvgImgItem = loadSvgImage(QString("%1.svg").arg(kBoxImageFilename));
    p_boxImgItem = p_boxSvgImgItem;

    p_surpriseSvgImgItem = loadSvgImage(QString("%1.svg").arg(kSurpriseImageFilename));
    p_surpriseImgItem = p_surpriseSvgImgItem;

    for(int i=0; i<NumBonusModes; i++)
    {
      p_modeSvgImgItem[i] = loadSvgImage(QString("%1.svg").arg(kModeImgFilenames[i]));
      p_modeImgItem[i] = p_modeSvgImgItem[i];
    }

    for(int i=0; i<NumBonusModes; i++)
    {
      p_boxModeSvgImgItem[i] = loadSvgImage(QString("%1.svg").arg(kBoxModeImgFilenames[i]));
      p_boxModeImgItem[i] = p_boxModeSvgImgItem[i];
    }
  }

  p_boxImgItem->setParentItem(this);
  p_boxImgItem->setVisible(false);

  p_surpriseImgItem->setParentItem(this);
  p_surpriseImgItem->setVisible(false);

  for(int i=0; i<NumBonusModes; i++)
  {
    p_modeImgItem[i]->setTransformOriginPoint(p_modeImgItem[i]->boundingRect().width() / 2.0,
                                              p_modeImgItem[i]->boundingRect().height() / 2.0);
    p_modeImgItem[i]->setParentItem(this);
    p_modeImgItem[i]->setVisible(false);

    p_boxModeImgItem[i]->setParentItem(this);
    p_boxModeImgItem[i]->setVisible(false);
  }

  m_imgInit = true;
}

void Bonus::pickMode()
{
  m_curMode = (BonusMode) (getRandom(0, NumBonusModes-1));

  /*!!! testing
  DrunkMode,
  SleepMode,
  SheepOnSpeedMode,
  TimeBonusMode,
  PowerBarkMode,
  m_curMode = PowerBarkMode;
   */

  if(getRandom(1, kSurpriseRatio) == 1)
    m_surprise = true;
  else
    m_surprise = false;
}

bool Bonus::modeIsValid() const
{
  if(m_curMode >= 0 && m_curMode < NumBonusModes)
    return true;

  return false;
}

void Bonus::delImg()
{
  if(p_boxPngImg != NULL)
    delete p_boxPngImg;
  if(p_boxSvgImgItem != NULL)
    delete p_boxSvgImgItem;
  if(p_surprisePngImg != NULL)
    delete p_surprisePngImg;
  if(p_surpriseSvgImgItem != NULL)
    delete p_surpriseSvgImgItem;

  if(p_boxPngImgItem != NULL)
    delete p_boxPngImgItem;
  if(p_surprisePngImgItem != NULL)
    delete p_surprisePngImgItem;

  for(int i=0; i<NumBonusModes; i++)
  {
    if(p_modePngImg[i] != NULL)
      delete p_modePngImg[i];
    if(p_modePngImgItem[i] != NULL)
      delete p_modePngImgItem[i];

    if(p_modeSvgImgItem[i] != NULL)
      delete p_modeSvgImgItem[i];

    if(p_boxModePngImg[i] != NULL)
      delete p_boxModePngImg[i];
    if(p_boxModePngImgItem[i] != NULL)
      delete p_boxModePngImgItem[i];

    if(p_boxModeSvgImgItem[i] != NULL)
      delete p_boxModeSvgImgItem[i];
  }
}

void Bonus::doBonusStuff()
{
#ifdef DEBUG_OUTPUT
  if(!modeIsValid())
  {
    qDebug("caught invalid mode %d!", m_curMode);
    return;
  }
#endif

  if(m_modeVisible)
  {
    if(gCurFrameNumber < m_curModeImageHideFrame)
    {
      qreal curScale = kMinShowModeImageScale + (kMaxShowModeImageScale - kMinShowModeImageScale) * m_curModeImageOffset;
      p_modeImgItem[m_curMode]->setScale(curScale);
      p_modeImgItem[m_curMode]->setOpacity(1.0 - m_curModeImageOffset);
      m_curModeImageOffset += m_curModeImageOffsetDelta;
    }
    else
    {
      hideMode();
    }
  }

  switch(m_curMode)
  {
    case NoBonusMode:
      return;

    case DrunkMode:
      break;

    case SleepMode:
      // Stop all sheep
      gNormalSheepSpeed = 0.0;
      foreach(Character *pSheep, gSheepList)
      {
        if(((Sheep*)pSheep)->rescued)
          continue;

        pSheep->changeSpeed(gNormalSheepSpeed);
        ((Sheep*)pSheep)->sleeping = true;

        //!!!p_sleepAnimation->setParentItem(this);
      }
      break;

    case SheepOnSpeedMode:
      // One time action - handled in pickRandomMode()
      break;

    case TimeBonusMode:
      // One time action - handled in pickRandomMode()
      return;
      break;

    case PowerBarkMode:
      // One time action - handled in pickRandomMode()
      return;
      break;

    default:
      qDebug("Current bonus mode invalid: %d", m_curMode);
      break;
  }
}

void Bonus::showBox()
{
#ifdef DEBUG_OUTPUT
  if(!modeIsValid())
  {
    qDebug("caught invalid mode %d!", m_curMode);
    return;
  }
#endif

  p_boxImgItem->setPos(m_colRect.x(), m_colRect.y());
  p_boxImgItem->setVisible(true);

  if(m_surprise)
  {
    /*!!!
    p_surpriseImgItem->setPos(m_colRect.x() + (m_colRect.width() - p_surpriseImgItem->boundingRect().width())/2.0,
                              m_colRect.y() + (m_colRect.height() - p_surpriseImgItem->boundingRect().height())/2.0);
    p_surpriseImgItem->setVisible(true);
    */
  }
  else
  {
    p_boxModeImgItem[m_curMode]->setPos(m_colRect.x() + (m_colRect.width() - p_boxModeImgItem[m_curMode]->boundingRect().width())/2.0,
                                        m_colRect.y() + (m_colRect.height() - p_boxModeImgItem[m_curMode]->boundingRect().height())/2.0);
    p_boxModeImgItem[m_curMode]->setVisible(true);
  }

  m_boxVisible = true;
}

void Bonus::hideBox()
{
  if(!m_imgInit)
    return;

  p_boxImgItem->setVisible(false);

  if(m_surprise)
  {
    //!!!p_surpriseImgItem->setVisible(false);
  }
  else
  {
    if(modeIsValid())
      p_boxModeImgItem[m_curMode]->setVisible(false);
  }

  m_boxVisible = false;
}

void Bonus::showMode()
{
#ifdef DEBUG_OUTPUT
  if(!modeIsValid())
  {
    qDebug("caught invalid mode %d!", m_curMode);
    return;
  }
#endif

  Level* pLevel = (Level*)p_level;

  m_curModeActivateFrame = gCurFrameNumber;

  m_curModeImageHideFrame = m_curModeActivateFrame + (kNumSecondsToShowModeImage * kFrameRate);

  m_curModeImageOffset = 0.0;
  m_curModeImageOffsetDelta = 1.0 / (qreal)(m_curModeImageHideFrame - m_curModeActivateFrame);

  switch(m_curMode)
  {
    case DrunkMode:
      gKeyCodeUp1    = kDefaultKeyCodeDown1;
      gKeyCodeRight1 = kDefaultKeyCodeLeft1;
      gKeyCodeDown1  = kDefaultKeyCodeUp1;
      gKeyCodeLeft1  = kDefaultKeyCodeRight1;

      gKeyCodeUp2    = kDefaultKeyCodeDown2;
      gKeyCodeRight2 = kDefaultKeyCodeLeft2;
      gKeyCodeDown2  = kDefaultKeyCodeUp2;
      gKeyCodeLeft2  = kDefaultKeyCodeRight2;

      if(gGraphicQuality >= HighQuality)
        p_scene->drunkMode(true);

      break;

    case SleepMode:
      break;

    case SheepOnSpeedMode:
      gNormalSheepSpeed *= kSheepOnSpeedFactor;
      foreach(Character *pSheep, gSheepList)
        pSheep->changeSpeed(gNormalSheepSpeed);
      break;

    case TimeBonusMode:
      // maximally refill to start time
      pLevel->m_bonusTime = qMin(kTimeBonus, pLevel->m_totalTime - pLevel->m_timeLeft);
      break;

    case PowerBarkMode:
      m_oldBarkReload = gBarkPowerReload;
      m_oldBarkMaxDist = gMaxBarkDist;
      m_oldBarkIntConst = gBarkIntensityConstant;
      m_oldSheepMaxSpeed = gMaxSheepSpeed;
      gBarkPowerReload = kPowerBarkReload;
      gMaxBarkDist = kPowerBarkDist;
      gBarkIntensityConstant = kPowerBarkIntensityConstant;
      gMaxSheepSpeed = kNormalDogSpeed;
      gSheepSlowDownValue = kPowerBarkSlowDown;
      break;

    default:
      break;
  }

  m_curModeDeactivateFrame = m_curModeActivateFrame + kBonusModeDuration[m_curMode] * kFrameRate;

  m_curModeImageMidPoint = QPointF(gDogList.at(0)->absMidPoint());

  p_modeImgItem[m_curMode]->setPos(m_curModeImageMidPoint - p_modeImgItem[m_curMode]->transformOriginPoint());

  m_modeVisible = true;
}

void Bonus::hideMode()
{
  m_modeVisible = false;

  if(modeIsValid())
    p_modeImgItem[m_curMode]->setVisible(false);
}

void Bonus::setNextBonusFrames()
{
  pickMode();

  // next frame in which a bonus appears (random)
  m_boxShowFrame = gCurFrameNumber + getRandom(kBonusMinDelay*kFrameRate, kBonusMaxDelay*kFrameRate);
  m_surpriseHideFrame = m_boxShowFrame + kSurpriseShowTime * kFrameRate;
}

void Bonus::checkForShow()
{
  if(!m_boxVisible && gCurFrameNumber < m_boxShowFrame)
  {
    //qDebug("There's no bonus (to be put) on stage, do nothing...");
    return;
  }

#ifdef DEBUG_OUTPUT
  if(!modeIsValid())
  {
    qDebug("caught invalid mode %d!", m_curMode);
    return;
  }
#endif

  Level* pLevel = (Level*)p_level;

  if(gCurFrameNumber == m_boxShowFrame)
  {
    QRectF levelRect(kFenceWidth, kFenceWidth,
                     pLevel->m_width - 2*kFenceWidth - p_boxImgItem->boundingRect().width(),
                     pLevel->m_height - 2*kFenceWidth - p_boxImgItem->boundingRect().height());

    QPointF viewToLevelMap = p_view->mapFromScene(pLevel->pos());
    QRectF viewRect(viewToLevelMap.x(), viewToLevelMap.y(),
                    gWindowWidth, gWindowHeight);

    // The surprise box may be placed in the intersection rectangle of the (usable) level rectangle and the view area
    // rectangle - in level coordinates.
    QRectF randomSpotRect = viewRect.intersected(levelRect);

    m_colRect = p_boxImgItem->boundingRect();
    while(1)
    {
      m_colRect.moveTo(getRandomF(randomSpotRect.x(), randomSpotRect.x() + randomSpotRect.width()),
                       getRandomF(randomSpotRect.y(), randomSpotRect.y() + randomSpotRect.height()));

      // check collisions with level elements
      bool collision = false;
      foreach(LevelElement *pLevelElement, gLevelElementList)
      {
        if(pLevelElement->getCollisionRect()->intersects(m_colRect))
        {
          collision = true;
          break;
        }
      }

      foreach(Character *pDog, gDogList)
      {
        if(pDog->getCollisionRect()->intersects(m_colRect))
        {
          collision = true;
          break;
        }
      }

      if(!collision)
        break;
		}
		
    showBox();

    m_bonusPickedUp = false;

    qDebug("bonus added: %3.1f, %3.1f", p_boxImgItem->x(), p_boxImgItem->y());
			
		return;
	}

  if(m_bonusPickedUp)
  {
    if(gCurFrameNumber == m_curModeDeactivateFrame)
    {
      qDebug("Bonus: Left %s", kBonusModeText[m_curMode]);

      setNextBonusFrames();
      m_bonusPickedUp = false;
      resetMode();
      return;
    }

    doBonusStuff();
  }
  else if(!m_bonusPickedUp && gCurFrameNumber == m_surpriseHideFrame)
	{
    qDebug("bonus removed: %3.1f, %3.1f", p_boxImgItem->x(), p_boxImgItem->y());

    hideBox();
    setNextBonusFrames();
    resetMode();
		return;
	}
  else if(!m_bonusPickedUp && m_boxVisible && gDogList.at(0)->getCollisionRect()->intersects(m_colRect))
	{
    qDebug("bonus picked up: %3.1f, %3.1f", p_boxImgItem->x(), p_boxImgItem->y());

    hideBox();
    showMode();

		m_bonusPickedUp = true;

		m_curModeActivateFrame = gCurFrameNumber;

	  qDebug("Bonus: Entered %s", kBonusModeText[m_curMode]);
	}
}

void Bonus::resetMode()
{
  if(!modeIsValid())
  {
    qDebug("current bonus mode is %d, nothing to reset...", m_curMode);
    return;
  }

  hideBox();
  hideMode();

	switch(m_curMode)
	{
		case DrunkMode:
      gKeyCodeUp1    = kDefaultKeyCodeUp1;
      gKeyCodeRight1 = kDefaultKeyCodeRight1;
      gKeyCodeDown1  = kDefaultKeyCodeDown1;
      gKeyCodeLeft1  = kDefaultKeyCodeLeft1;

      gKeyCodeUp2    = kDefaultKeyCodeUp2;
      gKeyCodeRight2 = kDefaultKeyCodeRight2;
      gKeyCodeDown2  = kDefaultKeyCodeDown2;
      gKeyCodeLeft2  = kDefaultKeyCodeLeft2;

      if(gGraphicQuality >= HighQuality)
        p_scene->drunkMode(false);

			break;
			
		case SleepMode:
      gNormalSheepSpeed = kNormalSheepSpeed;
      foreach(Character *pSheep, gSheepList)
      {
        if(((Sheep*)pSheep)->rescued)
					continue;
					
        ((Sheep*)pSheep)->sleeping = false;
        pSheep->changeSpeed(gNormalSheepSpeed);
      }
			break;

		case SheepOnSpeedMode:
      gNormalSheepSpeed = kNormalSheepSpeed;
      foreach(Character *pSheep, gSheepList)
        pSheep->changeSpeed(gNormalSheepSpeed);
			break;
			
		case TimeBonusMode:
			break;
			
		case PowerBarkMode:
      gBarkPowerReload       = m_oldBarkReload;
      gMaxBarkDist           = m_oldBarkMaxDist;
      gBarkIntensityConstant = m_oldBarkIntConst;
      gMaxSheepSpeed         = m_oldSheepMaxSpeed;
			break;

    case NoBonusMode:
		default:
		  qDebug("Current bonus mode invalid: %d", m_curMode);
      break;
	}
}


// ==============================================================================================
// implementations of virtual QGraphicsItem methods
// ==============================================================================================

QRectF Bonus::boundingRect() const
{
  if(m_boxVisible)
    return (p_boxImgItem->boundingRect());
  else if(m_modeVisible)
  {
    if(modeIsValid())
      return p_modeImgItem[m_curMode]->boundingRect();
    else
      qDebug("mode visible set even though cur mode %d invalid!", m_curMode);
  }

  return QRectF();
}

void Bonus::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
}
