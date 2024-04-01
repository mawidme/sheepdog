#ifndef BARK_WAVE_HPP_
#define BARK_WAVE_HPP_

#include <QtCore/QPointF>
#include <QtWidgets/QGraphicsEllipseItem>
#include <QtWidgets/QGraphicsBlurEffect>

class BarkWave
{
public:
  BarkWave(QPointF midPoint, qreal startRadius, qreal endRadius);
  ~BarkWave();

  bool update();

  QPointF m_midPoint;
  qreal m_curRadius;
  qreal m_maxRadius;
  qreal m_growthPerFrame;
  QGraphicsEllipseItem m_circle;

  QGraphicsBlurEffect m_blurEffect;

  bool m_finished;
};

#endif /* BARK_WAVE_HPP_ */
