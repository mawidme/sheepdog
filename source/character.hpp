#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include "level_element.hpp"
#include "globals.hpp"

#include "game_object.hpp"

#include <QGraphicsItem>
#include <QtSvg/QGraphicsSvgItem>


class Character : public GameObject//!!!, public QGraphicsItem
{
public:
  Character();
  ~Character();

  bool initImg();
  void initBasics();

  void moveByDeltaPoint(QPointF d);

  // TODO: reimplement QGraphicsItem::collidesWithItem () etc. here?!

  void initCollisions();
  QRectF* getCollisionRect();

  QRectF boundingRect() const;
  QPainterPath shape() const;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

  QPointF absMidPoint();
  QPointF relMidPoint();

  void setSpeedAndAngle(qreal speed, qreal angle);
  void changeAngle(qreal angle);
  void changeSpeed(qreal speed);
  qreal getSpeed();

  // unique character id
	long int charId;

#ifdef SHOW_COL_RECTS
	// item to draw collision rectangles
  QGraphicsRectItem m_colRectItem;
#endif

  qreal m_distToColRect;

  CharacterType m_characterType;

  QLineF m_speedVector;
  // Zero degrees is at the 3 o'clock position and positive values mean counter clock-wise direction.
  qreal  m_angle;

  QGraphicsItem       **p_img;
  QGraphicsPixmapItem **p_pngImg;
  QGraphicsSvgItem    **p_svgImg;
  long int m_curAnimationStep;
  long int m_numAnimationSteps;
  bool m_animationForward; // when the last animation step is reached, animation will be played backward

	// transform point (e.g. middle point for rotation) in character coordinates
	QPointF m_transformPoint;

  // Middle point for character's collision rectangle, in character coordinates.
	QPointF m_colRectMidPoint;

protected:
  // QGraphicsItem method
  void advance(int step);

  void updateAnimation();

  bool checkCollisionsForTranslation(QRectF *pCheckColRect);
  bool checkCollisions();

  void move(QLineF *pSpeedVector = NULL);

  QRectF m_boundingRect;
  qreal m_rotationOffset;

  QSizeF m_size;

  // COLLISION RECTANGLE - IN LEVEL COORDINATES!
  QRectF m_colRect;

  // character's middle point in...
  // a) relative character coordinates
  QPointF m_relativeMidPoint;
  // b) level coordinates (absolute)
  QPointF m_absoluteMidPoint;

  QPointF m_levelStartPos;

private:
  virtual void init();
};

#endif // CHARACTER_HPP
