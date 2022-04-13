#include "dog.hpp"
#include "sheep.hpp"
#include "global_lists.hpp"

#include "utils.hpp"

#include <cmath>

Dog::Dog(qreal sx, qreal sy)
{
  //qDebug("allocating dog 0x%x at (%.0f, %.0f)", (int)this, sx, sy);

  m_levelStartPos = QPointF(sx, sy);
  m_characterType = DogCharacter;
  m_rotationOffset = 90.0;

  m_imgFileName = QString("dog");
  if(gGraphicQuality < MedQuality)
    m_numAnimationSteps = 1;
  else
    m_numAnimationSteps = kNumAniStepsDog;

  init();
}

void Dog::init()
{
  initImg();

  setSpeedAndAngle(0, 0);

  m_curTargetPoint = kDogInitTargetPoint;

  m_distToColRect = (m_size.width() + m_size.height()) / 6.0;

  m_relativeMidPoint = QPointF(m_size.width() / 2.0, m_size.height() / 2.0);
  m_transformPoint = QPointF(m_size.width() / 2.0, m_size.height() - 26.0);
  m_colRectMidPoint = m_transformPoint;

  m_barkPower = kMaxBarkPower;

  initBasics();
}

// Returns true if barking was allowed.
bool Dog::bark()
{
  if(gSheepList.empty())
    return false;

  if(m_barkPower < kNeededBarkPower)
	{
#ifndef QT_NO_SOUND
		gNoBarkSound->play();
#endif

		return false;
	}
	
#ifndef QT_NO_SOUND
	long unsigned int rand = getRandom(0, kNumBarkSounds-1);
  gBarkSounds[rand]->play();
#endif

  m_barkPower -= kNeededBarkPower;

  if(((Sheep*)(gSheepList.at(0)))->sleeping)
		return true;

  foreach(Character *pSheep, gSheepList)
  {
    if(((Sheep *)pSheep)->sleeping)
      continue;

    if(((Sheep *)pSheep)->rescued)
			continue;
			
    long unsigned int dist = distBetweenPoints(QPointF(pSheep->x(), pSheep->y()),
                                               m_absoluteMidPoint);
		
    if(dist > gMaxBarkDist)
			continue;
			
    qreal newSpeed = min(gBarkIntensityConstant / dist * pSheep->getSpeed(), gMaxSheepSpeed);
    pSheep->changeSpeed(newSpeed);

    QLineF dogSheepLine(x(), y(), pSheep->x(), pSheep->y());
    pSheep->changeAngle(dogSheepLine.angle());
	}
	
#ifndef QT_NO_SOUND
	if(m_barkPower < 2*kNeededBarkPower && m_barkPower >= kNeededBarkPower)
    gBarkPowerCriticalSound->play();
#endif

	return true;
}

void Dog::zeroSpeedX()
{
  m_speedVector.setP2(QPointF(0, m_speedVector.y2()));
}

void Dog::zeroSpeedY()
{
  m_speedVector.setP2(QPointF(m_speedVector.x2(), 0));
}

void Dog::advance(int step)
{
  if(step == 0)
  {
    updateAnimation();
    return;
  }

  if(gPinchZooming)
    changeSpeed(0);

  // We are looking for future collisions (not current!) and therefore may return if speed is zero,
  // at least in the dog case. Sheep may be blocked and not be moving due to that
  if(getSpeed() > kZeroSpeedThreshold)
  {
    // only move dog until target reached (if mouse clicks resp. touchscreen are used for movement)
    if(m_curTargetPoint != kDogInitTargetPoint &&
       distBetweenPoints(m_curTargetPoint, m_absoluteMidPoint) <= gCurDogSpeed)
    {
      changeSpeed(0.0);

      m_curTargetPoint = kDogInitTargetPoint;
    }
    else
    {
      checkCollisions();
      move();
    }
  }

  // update bark power
  if(m_barkPower < kMaxBarkPower)
  {
#ifndef QT_NO_SOUND
		if(m_barkPower + gBarkPowerReload >= kMaxBarkPower)
      gBarkPowerFullSound->play();
#endif

    m_barkPower = qMin(m_barkPower + gBarkPowerReload, kMaxBarkPower);
  }
}
