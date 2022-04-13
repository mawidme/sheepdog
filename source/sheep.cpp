#include "sheep.hpp"

#include "globals.hpp"
#include "utils.hpp"

#include "dog.hpp"

#include "level.hpp"

#define _USE_MATH_DEFINES
#include <cmath>

Sheep::Sheep(qreal sx, qreal sy)
{
  //qDebug("allocating sheep 0x%x at (%.0f, %.0f)", (int)this, sx, sy);

  m_levelStartPos = QPointF(sx, sy);
  m_characterType = SheepCharacter;
  m_rotationOffset = 90.0;

  m_imgFileName = QString("sheep");
  if(gGraphicQuality < MedQuality)
    m_numAnimationSteps = 1;
  else
    m_numAnimationSteps = kNumAniStepsSheep;

  init();
}

void Sheep::init()
{
  rescued = false;
  blocked = false;
  sleeping = false;

  // bark in 10 + random[0, 10] seconds
  baaFrame = gCurFrameNumber + kFrameRate * (10 + getRandom(0, 10));

  initImg();

  setSpeedAndAngle(gNormalSheepSpeed, getRandomF(1.0, 360.0));

  m_distToColRect = (m_size.width() + m_size.height()) / 4.0;

  m_relativeMidPoint = QPointF(m_size.width() / 2.0, m_size.height() / 2.0);
  m_transformPoint = QPointF(m_size.width() / 2.0, m_size.height() / 2.0);
  m_colRectMidPoint = m_transformPoint;

  initBasics();
}

void Sheep::init(void *pLevel)
{
  p_level = pLevel;
}

void Sheep::advance(int step)
{
  if(step == 0)
  {
    updateAnimation();
    return;
  }

  // already saved sheep aren't visible anymore ;-)
  if(rescued || sleeping)
    return;

#ifndef QT_NO_SOUND
	// random baa
  if(gCurFrameNumber == baaFrame)
  {
    gSheepSound[getRandom(0, kNumSheepSounds-1)]->play();
    // next bark in 10 + random[0, 10] seconds
    baaFrame = gCurFrameNumber + kFrameRate * (10 + getRandom(0, 10));
  }
#endif // !QT_NO_SOUND

  if(blocked)
  {
    // blocked sheep: check for way out every few frames
    if(gCurFrameNumber % kBlockedSheepNumFrameSkip == 0)
    {
      blocked = false;
      changeSpeed(gNormalSheepSpeed);
    }
    else
      return;
  }

  refreshSpeed();

  // If a collision occurs, new position and speed/angle will be set in checkCollisions().
  // Otherwise, move after collision check.
  if(checkCollisions())
  {
    if(getSpeed() == 0.0)
      blocked = true;
  }
  else
    move();

  // return if sheep has been rescued
  if(rescued)
    return;
}

void Sheep::refreshSpeed()
{
  if(getSpeed() > gNormalSheepSpeed)
    changeSpeed(max(gNormalSheepSpeed, getSpeed() - gSheepSlowDownValue));
}

/*!!!
void Sheep::turnAround()
{
  for(int i=0; i<m_numAnimationSteps; i++)
    changeAngle( ((int)p_img[i]->rotation() + 90) % 360 );
}
*/

void Sheep::goToExit()
{
#ifndef QT_NO_SOUND
 gSheepRescuedSound[getRandom(0, kNumSheepRescuedSounds-1)]->play();
#endif

  rescued = true;
  setVisible(false);

  ((Level*)p_level)->minusOneSheep();

#ifdef DEBUG
  qdebug("sheep %d exit!", charId);
  qdebug("numLeftSheep: %d", m_numLeftSheep);
  qdebug("total sheep: %d", m_numTotalSheep);
#endif

  return;
}
