#include "character.hpp"
#include "sheep.hpp"
#include "dog.hpp"

#include "global_lists.hpp"

#include "utils.hpp"

#include <cmath>

#include <QFile>

Character::Character() : p_img(NULL),
                         p_pngImg(NULL),
                         p_svgImg(NULL),
                         m_curAnimationStep(-1),
                         m_numAnimationSteps(-1),
                         m_animationForward(true)
{
}

Character::~Character()
{
  if(p_pngImg != NULL)
  {
    for(int i=0; i<m_numAnimationSteps; i++)
      delete p_pngImg[i];

    delete p_pngImg;
  }

  if(p_svgImg != NULL)
  {
    for(int i=0; i<m_numAnimationSteps; i++)
      delete p_svgImg[i];

    delete p_svgImg;
  }

  if(p_img != NULL)
    delete p_img;
}

void Character::init()
{
}

bool Character::initImg()
{
  if(m_numAnimationSteps == -1)
  {
    qDebug("set m_numAnimationSteps before initImg()!");
    return false;
  }
  if(m_characterType != DogCharacter && m_characterType != SheepCharacter)
  {
    qDebug("invalid character type %d!", m_characterType);
    return false;
  }

  if(gGraphicQuality < HighQuality)
    m_imgType = PngImgType;
  else
    m_imgType = SvgImgType;

  if(p_img == NULL)
    p_img = new QGraphicsItem*[m_numAnimationSteps];

  if(m_imgType == PngImgType)
  {
    if(gPngCharacterImg[m_characterType] == NULL)
    {
      gPngCharacterImg[m_characterType] = new QPixmap*[m_numAnimationSteps];
      if(gGraphicQuality < MedQuality)
      {
        // no animations for low graphics quality!
        QString filename;
        if(m_characterType == DogCharacter)
          filename = QString("img/dog04.png");
        else if(m_characterType == SheepCharacter)
          filename = QString("img/sheep01.png");

        gPngCharacterImg[m_characterType][0] = loadPngImage(filename);
      }
      else
      {
        for(int i=0; i<m_numAnimationSteps; i++)
        {
          QString filename = QString("img/%1%2.png").arg(m_imgFileName).arg(i+1, 2, 10, QChar('0'));
          gPngCharacterImg[m_characterType][i] = loadPngImage(filename);
        }
      }
    }

    if(p_pngImg == NULL)
    {
      p_pngImg = new QGraphicsPixmapItem*[m_numAnimationSteps];
      for(int i=0; i<m_numAnimationSteps; i++)
      {
        p_pngImg[i] = new QGraphicsPixmapItem(*gPngCharacterImg[m_characterType][i]);
        p_pngImg[i]->setParentItem(this);

        p_img[i] = p_pngImg[i];
      }
    }
  }
  else if(m_imgType == SvgImgType)
  {
    if(gSvgCharacterRenderer[m_characterType] == NULL)
    {
      gSvgCharacterRenderer[m_characterType] = new QSvgRenderer*[m_numAnimationSteps];
      if(gGraphicQuality < MedQuality)
      {
        QString filename;
        if(m_characterType == DogCharacter)
          filename = QString("img/dog04.svg");
        else if(m_characterType == SheepCharacter)
          filename = QString("img/sheep01.svg");
        loadSvgImage(&gSvgCharacterRenderer[m_characterType][0], filename);
      }
      else
      {
        for(int i=0; i<m_numAnimationSteps; i++)
        {
          QString filename = QString("img/%1%2.svg").arg(m_imgFileName).arg(i+1, 2, 10, QChar('0'));
          loadSvgImage(&gSvgCharacterRenderer[m_characterType][i], filename);
        }
      }
    }

    if(p_svgImg == NULL)
    {
      p_svgImg = new QGraphicsSvgItem*[m_numAnimationSteps];
      for(int i=0; i<m_numAnimationSteps; i++)
      {
        p_svgImg[i] = new QGraphicsSvgItem();
        p_svgImg[i]->setSharedRenderer(gSvgCharacterRenderer[m_characterType][i]);

        p_img[i] = p_svgImg[i];
      }
    }
  }
  else
  {
    qDebug("unsupported image type %u for character!", m_imgType);
    return false;
  }

  m_curAnimationStep = 0;
  for(int i=0; i<m_numAnimationSteps; i++)
  {
    if(i > 0)
      p_img[i]->setVisible(false);
    else
      p_img[i]->setVisible(true);
  }

  // all images of the sequence have to be of the same size!
  m_size = p_img[0]->boundingRect().size();

  m_imgInit = true;

  return true;
}

void Character::initBasics()
{
  if(m_numAnimationSteps == -1)
  {
    qDebug("set m_numAnimationSteps before initImg()!");
    return;
  }

  if(!m_imgInit)
  {
    qDebug("call initImg() before initBasics()!");
    return;
  }

  qreal x = m_levelStartPos.x();
  qreal y = m_levelStartPos.y();

  qreal boundingRectWidth;
  if(m_size.width() > m_size.height())
    boundingRectWidth = m_size.width();
  else
    boundingRectWidth = m_size.height();
  m_boundingRect = QRectF(m_relativeMidPoint.x() - (boundingRectWidth / 2.0), m_relativeMidPoint.y() - (boundingRectWidth / 2.0),
                          boundingRectWidth, boundingRectWidth);

  m_topLeftPos = QPointF(x - m_boundingRect.width()/2.0, y - m_boundingRect.height()/2.0);
  m_bottomRightPos = QPointF(x + m_boundingRect.width()/2.0, y + m_boundingRect.height()/2.0);

  charId = gCharIdCounter;
  gCharIdCounter++;

  m_absoluteMidPoint = QPointF(x, y);
  for(int i=0; i<m_numAnimationSteps; i++)
  {
    setTransformOriginPoint(m_transformPoint);

    // set all animation steps to invisible except first one
    if(i != m_curAnimationStep)
      p_img[i]->setVisible(false);
  }

  QPointF absDrawCoordinates = m_absoluteMidPoint - m_relativeMidPoint;
  setPos(absDrawCoordinates);

  initCollisions();

  setVisible(true);
}

void Character::initCollisions()
{
  if(gColCheckStepDiff > 2 * m_distToColRect)
    gColCheckStepDiff = 2 * m_distToColRect - 1.0;

  m_colRect.setRect(x() + m_colRectMidPoint.x() - m_distToColRect,
                    y() + m_colRectMidPoint.y() - m_distToColRect,
                    2 * m_distToColRect,
                    2 * m_distToColRect);

#ifdef SHOW_COL_RECTS
  m_colRectItem.setRect(m_colRect);
  m_colRectItem.setPen(kInactiveColRectPen);
  m_colRectItem.setBrush(kInactiveColRectBrush);
#endif
}

QRectF* Character::getCollisionRect()
{
  return &m_colRect;
}

// Returns true if the given rectangle collides with any level element or character.
bool Character::checkCollisionsForTranslation(QRectF *pCheckColRect)
{
  foreach(LevelElement *pLevelElement, gLevelElementList)
  {
    QRectF *pLevelElementColRect = pLevelElement->getCollisionRect();
    if(pLevelElementColRect == NULL ||
        (m_characterType == DogCharacter && pLevelElement->getElemType() == ExitLevelElement) )
    {
      // some level elements have no collision area (e.g. grass land) and
      // dogs do not collide with exit elements.
      continue;
    }

    if(pLevelElementColRect->intersects(*pCheckColRect))
      return true;
  }

  QList<Character *> *p_check_list;
  if(m_characterType == DogCharacter)
  {
    // TODO: check dog-dog collisions in the future -> e.g. arrays of character lists with for loop here

    p_check_list = &gSheepList;
  }
  else if(m_characterType == SheepCharacter)
  {
    p_check_list = &gDogList;
  }
  else
  {
    qDebug("invalid character type %d!", m_characterType);
    exit(EXIT_FAILURE);
  }

  for(int i=0; i<p_check_list->size(); i++)
  {
    Character *p_character = p_check_list->at(i);

    // don't check for collision with oneself
    if(charId == p_character->charId)
      continue;

    if(!p_character->isVisible())
      continue;

    if(p_character->getCollisionRect()->intersects(*pCheckColRect))
      return true;
  }

  return false;
}

// Returns true on collision
bool Character::checkCollisions()
{
  // TODO: check dog-dog collisions in the future
  QList<Character *> *p_otherCharList;
  if(m_characterType == SheepCharacter)
    p_otherCharList = &gDogList;
  else if(m_characterType == DogCharacter)
    p_otherCharList = &gSheepList;
  else
  {
    qDebug("Invalid character type %d!", m_characterType);
    return false;
  }

  // Sheep are not moving as fast as gColCheckStepDiff most of the time.
  bool oneStepOnly = false;
  qreal startWalkDist = gColCheckStepDiff;
  if(startWalkDist > m_speedVector.length())
  {
    startWalkDist = m_speedVector.length();
    oneStepOnly = true;
  }

  // Last ok position before collision. Again assume whole distance may be walked.
  QRectF lastOkColRect = m_colRect.translated(m_speedVector.dx(), m_speedVector.dy());
  qreal  lastOkWalkDist = m_speedVector.length();

  QRectF *p_nearestOtherColRect = NULL;

  // set on collision with level element or character
  bool collided = false;
  // will be set before last "walk distance check loop"
  bool breakOnNextDist = false;
  // Cache previous step's walking distance as it equals the last ok walking distance if collision is detected.
  qreal prevWalkDist = 0.0;
  qreal curWalkDist = startWalkDist;
  while(1)
  {
    QLineF curWalkDistVector(m_speedVector);
    curWalkDistVector.setLength(curWalkDist);
    QRectF colCheckRect = m_colRect.translated(curWalkDistVector.dx(), curWalkDistVector.dy());

    // first check for collisions with level elements, then check characters
    bool checkingElements = true;
    LevelElement *p_level_element = NULL;
    Character *p_character = NULL;
    int list_counter = 0;
    while(1)
    {
      QRectF *p_otherColRect = NULL;

      if(checkingElements && list_counter == gLevelElementList.size())
      {
        // stop checking level element list and switch to character list
        checkingElements = false;
        list_counter = 0;
        p_level_element = NULL;
        p_otherColRect = NULL;
      }

      if(checkingElements)
      {
        p_level_element = gLevelElementList.at(list_counter++);
        p_otherColRect = p_level_element->getCollisionRect();
      }
      else
      {
        if(list_counter == p_otherCharList->size())
          break;

        p_character = p_otherCharList->at(list_counter++);

        // Ignore characters that have already disappeared (like rescued sheep).
        if(!p_character->isVisible())
          continue;

        p_otherColRect = p_character->getCollisionRect();
      }

      if(p_otherColRect == NULL ||
         (checkingElements && m_characterType == DogCharacter && p_level_element->getElemType() == ExitLevelElement) )
      {
        // some level elements have no collision area (e.g. grass land) and
        // dogs do not collide with exit elements.
        continue;
      }

      if(p_otherColRect->intersects(colCheckRect))
      {
        if(oneStepOnly || prevWalkDist < lastOkWalkDist)
        {
          // Collision with current level element occurs sooner...if set before anyway...
          p_nearestOtherColRect = p_otherColRect;

          lastOkWalkDist = prevWalkDist;

          curWalkDistVector.setLength(lastOkWalkDist);
          lastOkColRect = m_colRect.translated(curWalkDistVector.dx(), curWalkDistVector.dy());
        }

        if(checkingElements && m_characterType == SheepCharacter && p_level_element->getElemType() == ExitLevelElement)
        {
          // sheep has reached an exit
          Sheep *p_sheep = (Sheep *) this;
          p_sheep->goToExit();
          return true;
        }

#ifdef DEBUG_COLLISION_CHECKING
        if(checkingElements)
          qDebug("%s estimated collision: elmt (%2.1f, %2.1f) type %d after %2.1fpx, will walk %2.1fpx, speed %2.1f",
                 m_characterType == DogCharacter ? "dog" : "sheep",
                 p_level_element->pos().x(), p_level_element->pos().y(),
                 p_level_element->getElemType(), curWalkDist, lastOkWalkDist, m_speedVector.length());
        else
          qDebug("%s estimated collision with character: after %2.1fpx, will walk %2.1fpx, speed %2.1f",
                 m_characterType == DogCharacter ? "dog" : "sheep", curWalkDist, lastOkWalkDist, m_speedVector.length());
#endif /* DEBUG_COLLISION_CHECKING */

        // Collision occurred, no need to do further checks...
        collided = true;
        break;
      }
    }

    if(collided)
      break;

    prevWalkDist = curWalkDist;

    if(breakOnNextDist || curWalkDist >= m_speedVector.length())
    {
      break;
    }
    else
    {
      // Speed is probably not a multiple of gColCheckStepDiff, so make sure the speed value itself
      // is used for collision check.
      if(curWalkDist + gColCheckStepDiff >= m_speedVector.length())
      {
        breakOnNextDist = true;
        curWalkDist = m_speedVector.length();
      }
      else
      {
        curWalkDist += gColCheckStepDiff;
      }
    }
  }

  if(p_nearestOtherColRect != NULL)
  {
    QLineF lastOkWalkDistVector = QLineF(m_speedVector);
    lastOkWalkDistVector.setLength(lastOkWalkDist);

    // Find exact possible walking distance before collision if the defined resolution demands it.
    if(m_speedVector.length() > kSpeedResolution)
    {
      // Cache previous step's walking distance as it equals the last ok walking distance if collision is detected.
      qreal prevWalkDist = 0.0;
      for(qreal curWalkDist=lastOkWalkDist; curWalkDist<=lastOkWalkDist+gColCheckStepDiff; curWalkDist += kSpeedResolution)
      {
        lastOkWalkDistVector = QLineF(m_speedVector);
        lastOkWalkDistVector.setLength(curWalkDist);
        QRectF colCheckRect = m_colRect.translated(lastOkWalkDistVector.dx(), lastOkWalkDistVector.dy());

        if(p_nearestOtherColRect->intersects(colCheckRect))
        {
          // Exact possible distance found
          lastOkWalkDist = prevWalkDist;

#ifdef DEBUG_COLLISION_CHECKING
          qDebug("%s exact collision: after %2.1fpx, will walk %2.1fpx, speed %2.1f",
                 m_characterType == DogCharacter ? "dog" : "sheep", curWalkDist, lastOkWalkDist, m_speedVector.length());
#endif

          lastOkWalkDistVector.setLength(lastOkWalkDist);
          lastOkColRect = m_colRect.translated(lastOkWalkDistVector.dx(), lastOkWalkDistVector.dy());
          break;
        }

        prevWalkDist = curWalkDist;
      }
    }

    if(m_characterType == DogCharacter)
    {
      changeSpeed(0.0);
      move(&lastOkWalkDistVector);
    }
    else if(m_characterType == SheepCharacter)
    {
      // reset speed to normal after collision
      changeSpeed(gNormalSheepSpeed);
      qreal remainingWalkDist = m_speedVector.length();

      // Look for way out in steps of 90°. If escaping direction is found, walk that way
      // plus a random number

      int tmpAngle = floor(m_angle / 90.0) * 90;

      QLineF walkVectorsToCheck[kNumWalkDirections];
      for(unsigned int i=0; i<kNumWalkDirections; i++)
      {
        // Initialize possible "escape speed vectors" to minimum check length.
        walkVectorsToCheck[i].setLine(0.0, 0.0, remainingWalkDist, 0.0);
        walkVectorsToCheck[i].setAngle(tmpAngle);
        tmpAngle = (tmpAngle + 90) % 360;
      }

      // Number of 90°-directions resulting in a collision.
      int numColDirs = 0;
      bool colDir[kNumWalkDirections];
      for(unsigned int i=0; i<kNumWalkDirections; i++)
      {
        QRectF checkColRect = lastOkColRect.translated(walkVectorsToCheck[i].p2());

        colDir[i] = checkCollisionsForTranslation(&checkColRect);
        if(colDir[i])
          numColDirs++;
      }

      int firstNoColDir = 0;
      while(colDir[firstNoColDir])
        firstNoColDir++;

      qreal newAngle = 0.0;
      if(numColDirs == 1)
      {
        // If next direction (+90°) leads to a collision, subtract a random number. Otherwise, add a
        // random number.
        if(colDir[(firstNoColDir + 1) % 4])
          newAngle = walkVectorsToCheck[firstNoColDir].angle() - (getRandom(0, 45));
        else
          newAngle = walkVectorsToCheck[firstNoColDir].angle() + (getRandom(0, 45));
      }
      else if(numColDirs == 2)
      {
        if(!colDir[(firstNoColDir + 1) % 4] || !colDir[(firstNoColDir + 3) % 4])
        {
          // Directions with no collision are adjacent (i.e. differ by 90°) and next 90° step angle
          // with no collision is in positive direction.
          newAngle = walkVectorsToCheck[firstNoColDir].angle() + (getRandom(0, 45));
        }
        else if(!colDir[(firstNoColDir + 3) % 4])
        {
          // Directions with no collision are adjacent (i.e. differ by 90°) and next 90° step angle
          // with no collision is in negative direction.
          newAngle = walkVectorsToCheck[firstNoColDir].angle() - (getRandom(0, 45));
        }
        else
        {
          // Directions with no collision differ by 180°,
          // make a random choice between the two remaining directions.
          if(getRandom(0, 1) == 0)
            newAngle = walkVectorsToCheck[firstNoColDir].angle();
          else
            newAngle = walkVectorsToCheck[firstNoColDir + 2].angle();
        }
      }
      else if(numColDirs == 3)
      {
        // There's only one way out, no (random) choice!
        newAngle = walkVectorsToCheck[firstNoColDir].angle();
      }
      else
      {
        qDebug("sheep blocked! (number of collisions = %d)", numColDirs);
        changeSpeed(0.0);
        return true;
      }

#ifdef DEBUG_COLLISION_CHECKING
      qDebug("sheep: %d collision(s), old angle = %lf, new angle = %lf", numColDirs, m_angle, newAngle);
#endif

      QLineF helpVector(0.0, 0.0, 1.0, 0.0);
      helpVector.setAngle(newAngle);
      helpVector.setLength(remainingWalkDist);

      QLineF tmpMoveVector(QPointF(0.0, 0.0), lastOkWalkDistVector.p2() + helpVector.p2());
      changeAngle(newAngle);
      move(&tmpMoveVector);
    }

    return true;
  }

  return false;
}

void Character::moveByDeltaPoint(QPointF d)
{
  QLineF line(QPointF(0,0), d);
  move(&line);
  m_levelStartPos += d;
}

// If a vector is given as argument (p_speedVector != NULL), only walk that distance.
void Character::move(QLineF *pSpeedVector)
{
  QLineF *p_usedSpeedVector = &m_speedVector;

  if(pSpeedVector != NULL)
    p_usedSpeedVector = pSpeedVector;

  if(p_usedSpeedVector->isNull())
    return ;

  moveBy(p_usedSpeedVector->dx(), p_usedSpeedVector->dy());

  m_absoluteMidPoint += p_usedSpeedVector->p2();

  m_topLeftPos += p_usedSpeedVector->p2();
  m_bottomRightPos += p_usedSpeedVector->p2();

  // Update collision area rectangle position:
  // More efficient to do that calculation here once than in each collision check separately!
  m_colRect.translate(p_usedSpeedVector->dx(), p_usedSpeedVector->dy());

#ifdef SHOW_COL_RECTS
  m_colRectItem.translate(p_usedSpeedVector->dx(), p_usedSpeedVector->dy());
#endif
}

QPointF Character::absMidPoint()
{
  return m_absoluteMidPoint;
}

QPointF Character::relMidPoint()
{
  return m_relativeMidPoint;
}

qreal Character::getSpeed()
{
  return m_speedVector.length();
}

void Character::changeSpeed(qreal speed)
{
  if(m_speedVector.x2() == 0 && m_speedVector.y2() == 0)
  {
    //reinit speed vector if speed was zero before
    m_speedVector.setPoints(QPointF(0.0, 0.0), QPointF(1.0, 1.0));
    changeAngle(m_angle);
  }
  m_speedVector.setLength(speed);
}

void Character::changeAngle(qreal angle)
{
  while (angle < 0.0)
    angle += 360.0;
  while (angle > 360.0)
    angle -= 360.0;
  m_angle = angle;
  m_speedVector.setAngle(m_angle);

  for(int i=0; i<m_numAnimationSteps; i++)
    setRotation(-m_angle - m_rotationOffset);
}

void Character::setSpeedAndAngle(qreal speed, qreal angle)
{
  m_speedVector.setPoints(QPointF(0.0, 0.0), QPointF(1.0, 1.0));
  m_speedVector.setLength(speed);
  changeAngle(angle);
}

void Character::updateAnimation()
{
  if(m_numAnimationSteps <= 1)
    return;

  p_img[m_curAnimationStep]->setVisible(false);

  if(m_animationForward)
  {
    if(m_curAnimationStep == m_numAnimationSteps-1)
    {
      m_curAnimationStep--;
      m_animationForward = false;
    }
    else
    {
      m_curAnimationStep++;
    }
  }
  else
  {
    if(m_curAnimationStep == 0)
    {
      m_curAnimationStep++;
      m_animationForward = true;
    }
    else
    {
      m_curAnimationStep--;
    }
  }

  p_img[m_curAnimationStep]->setVisible(true);
}


// ==============================================================================================
// implementations of virtual QGraphicsItem methods
// ==============================================================================================

void Character::advance(int step)
{
  qDebug("Char. advance");
}

void Character::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                      QWidget *widget)
{
  // We don't have to paint p_img here explicitly as it has been added to the scene before
  // and the whole scene is painted anyway...

#ifdef SHOW_COL_RECTS
  m_colRectItem.paint(painter, option, widget);
#endif /* SHOW_COL_RECTS */
}

QPainterPath Character::shape() const
{
  QPainterPath path;
  path.addRect(m_boundingRect);
  return path;
}

QRectF Character::boundingRect() const
{
  return m_boundingRect;
}
