#include "bark_wave.hpp"

#include "globals.hpp"

const qreal kBarkWaveDuration = 0.3; // [s]

BarkWave::BarkWave(QPointF midPoint, qreal startRadius, qreal endRadius)
{
  m_midPoint = midPoint;
  m_curRadius = startRadius;
  m_maxRadius = endRadius;

  m_growthPerFrame = (endRadius - startRadius) / (kBarkWaveDuration * (qreal)kFrameRate);

  if(gGraphicQuality >= HighQuality)
  {
    m_blurEffect.setBlurRadius(10.0);
    m_blurEffect.setEnabled(true);
    m_circle.setGraphicsEffect(&m_blurEffect);
  }
  m_circle.setPos(m_midPoint.x() - m_curRadius, m_midPoint.y() - m_curRadius);
  m_circle.setRect(0.0, 0.0, 2*m_curRadius, 2*m_curRadius);
  m_circle.setPen(gBarkWavePen);
  m_circle.setBrush(gBarkWaveBrush);

  m_finished = false;
}

BarkWave::~BarkWave()
{
}

// Returns true if wave is still active
bool BarkWave::update()
{
  if(m_curRadius > m_maxRadius)
  {
    m_circle.setParentItem(NULL);
    m_finished = true;
    return false;
  }

  m_circle.setPos(m_midPoint.x() - m_curRadius, m_midPoint.y() - m_curRadius);
  m_circle.setRect(0.0, 0.0, 2*m_curRadius, 2*m_curRadius);
  m_curRadius += m_growthPerFrame;

  return true;
}
