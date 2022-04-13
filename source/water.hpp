#ifndef WATER_HPP
#define WATER_HPP

#include "level_element.hpp"
#include "level_types.hpp"

#include <QPointF>

static const qreal kPenWidth = 2.0;

class Water : public LevelElement
{
public:
  Water(QPointF point1, QPointF point2, long unsigned int style) : p_pathItem(NULL)
	{
    if(fabs(point1.x() - point2.x()) < kMinWaterSize ||
       fabs(point1.y() - point2.y()) < kMinWaterSize)
    {
      qDebug("water too small!");
      return;
    }

    m_topLeftPos = QPointF(qMin(point1.x(), point2.x()),
                           qMin(point1.y(), point2.y()));
    m_bottomRightPos = QPointF(qMax(point1.x(), point2.x()),
                               qMax(point1.y(), point2.y()));

    //qDebug("allocating water 0x%x at (%.0f, %.0f)", (unsigned int)this, m_topLeftPos.x(), m_topLeftPos.y());

    m_elemType = WaterLevelElement;

    qreal width = m_bottomRightPos.x() - m_topLeftPos.x();
    qreal height = m_bottomRightPos.y() - m_topLeftPos.y();

    initColRect(m_topLeftPos.x(), m_topLeftPos.y(), width, height);

    m_layer = 1;

    // add graphics
    m_imgStyle = style - 1;
    m_imgType = CustomImgType;

    m_path.addRoundedRect(0, 0, width, height, kMinWaterSize / 2.0, kMinWaterSize / 2.0);
    setPos(m_topLeftPos.x(), m_topLeftPos.y());

    QPen pen(QColor(0, 0, 0, 0));
    QBrush brush(QColor(0, 0, 170));
    p_pathItem = new QGraphicsPathItem(m_path, this);
    p_pathItem->setPen(pen);
    p_pathItem->setBrush(brush);

    m_boundingRect = QRectF(m_topLeftPos.x(), m_topLeftPos.y(), width, height);
  }

  ~Water()
  {
    if(p_pathItem != NULL)
      delete p_pathItem;
  }

private:
  QGraphicsPathItem *p_pathItem;
  QPainterPath m_path;
};

#endif // WATER_HPP
