#include "game_object.hpp"

GameObject::GameObject() : m_imgInit(false), m_addedToLevel(false)
{
}

GameObject::~GameObject()
{
}

QPointF GameObject::getTopLeftPos()
{
  return m_topLeftPos;
}

QPointF GameObject::getBottomRightPos()
{
  return m_bottomRightPos;
}

bool GameObject::added()
{
  return m_addedToLevel;
}

void GameObject::setAdded()
{
  m_addedToLevel = true;
}

QString GameObject::getImageSuffix()
{
  QString suffix;

  switch(m_imgType)
  {
  case PngImgType:
  case PngImgListType:
    suffix = QString("png");
    break;
  case SvgImgType:
    suffix = QString("svg");
    break;
  case CustomImgType:
    qDebug("No need to load graphics for custom element!");
    break;
  default:
    qDebug("Cannot load graphics for invalid image type %d!", m_imgType);
    break;
  }

  return suffix;
}


// ==============================================================================================
// implementations of virtual QGraphicsItem methods
// ==============================================================================================

void GameObject::paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget)
{
  qDebug("GameObject::paint() called!");
}

QRectF GameObject::boundingRect() const
{
  qDebug("GameObject::boundingRect() called!");
  return QRectF();
}

void GameObject::advance(int step)
{
  qDebug("GameObject::advance() called!");
}
