#ifndef FENCE_HPP
#define FENCE_HPP

#include "level_element.hpp"
#include "level_types.hpp"

#include <cmath>

const qreal kMinFenceLen = 60.0;

class Fence : public LevelElement
{
public:
  ~Fence()
  {
    while(!m_poleList.isEmpty())
    {
      QGraphicsEllipseItem *p_pole = m_poleList.front();
      m_poleList.pop_front();
      delete p_pole;
    }
  }

  // arguments are middlepoint of first pole and middlepoint of last pole!
  Fence(qreal sxStart, qreal syStart, qreal sxEnd, qreal syEnd)
  {
    m_elemType = FenceLevelElement;

    m_imgType = CustomImgType;

    // characters may put their heads through the gaps in the fence (and therefore are on a lower layer)
    m_layer = 3;

    if(fabs(sxEnd - sxStart) < fabs(syEnd - syStart))
    {
      m_vertical = true;

      qreal distExtPoleMids = abs(syEnd - syStart);     // distance between first and last pole middle point
      qreal height = distExtPoleMids + kFenceWidth;
      qreal xStart, yStart;
      if(syStart < syEnd)
        yStart = syStart;
      else
        yStart = syEnd;

      xStart = sxStart;

      m_topLeftPos = QPointF(xStart - kFenceWidth/2.0, yStart - kFenceWidth/2.0);
      m_bottomRightPos = QPointF(m_topLeftPos.x() + kFenceWidth, m_topLeftPos.y() + height);
      //qDebug("allocating vert.fence 0x%x at (%.0f, %.0f)", (unsigned int)this, xStart, yStart);

      qreal d = (kFenceWidth-kFenceSlatWidth)/2.0; // distance from top left corner (the same for vert. & hor.)
      initColRect(xStart + d, yStart + d, kFenceSlatWidth, height);

      // add graphics
      m_slatRect.setPos(m_topLeftPos.x() + (kFenceWidth - kFenceSlatWidth)/2.0, yStart);
      m_slatRect.setRect(0.0, 0.0, kFenceSlatWidth, distExtPoleMids);
      m_slatRect.setBrush(gFenceSlatBrush);
      m_slatRect.setPen(gFenceSlatPen);
      m_slatRect.setParentItem(this);

      // divide fence into parts with a minimum length of: 2 * kTileHeight
      int numSlats = qMax(1, (int)(distExtPoleMids / kMinFenceLen));
      qreal yStep = distExtPoleMids / (qreal)numSlats;
      qreal y = yStart;
      for(int i=0; i<=numSlats; i++)
      {
        QGraphicsEllipseItem *p_pole = new QGraphicsEllipseItem(xStart - kFenceWidth/2.0, y - kFenceWidth/2.0,
                                                                kFenceWidth, kFenceWidth);
        p_pole->setBrush(gFencePoleBrush);
        p_pole->setPen(gFencePolePen);
        p_pole->setParentItem(this);
        m_poleList.append(p_pole);

        y += yStep;
      }

      m_boundingRect = QRectF(m_topLeftPos.x(), m_topLeftPos.y(), kFenceWidth, height);
    }
    else
    {
      m_vertical = false;

      qreal distExtPoleMids = abs(sxEnd - sxStart);     // distance between first and last pole middle point
      qreal width = distExtPoleMids + kFenceWidth;
      qreal xStart, yStart;
      if(sxEnd > sxStart)
        xStart = sxStart;
      else
        xStart = sxEnd;

      yStart = syStart;

      m_topLeftPos = QPointF(xStart - kFenceWidth/2.0, yStart - kFenceWidth/2.0);
      m_bottomRightPos = QPointF(m_topLeftPos.x() + width, m_topLeftPos.y() + kFenceWidth);
      //qDebug("allocating hor.fence 0x%x at (%.0f, %.0f)", (unsigned int)this, xStart, yStart);

      //!!! change to slat rect?
      //initColRect(xStart, yStart, width, kFenceWidth);
      qreal d = (kFenceWidth-kFenceSlatWidth)/2.0; // distance from top left corner (the same for vert. & hor.)
      initColRect(xStart + d, yStart + d,
                  width, kFenceSlatWidth);

      // add graphics
      m_imgType = CustomImgType;
      m_slatRect.setPos(xStart, m_topLeftPos.y() + (kFenceWidth - kFenceSlatWidth)/2.0);
      m_slatRect.setRect(0.0, 0.0, distExtPoleMids, kFenceSlatWidth);
      m_slatRect.setBrush(gFenceSlatBrush);
      m_slatRect.setPen(gFenceSlatPen);
      m_slatRect.setParentItem(this);

      // divide fence into parts with a minimum length of kMinFenceLen
      int numSlats = qMax(1, (int)(distExtPoleMids / kMinFenceLen));
      qreal xStep = distExtPoleMids / (qreal)numSlats;
      qreal x = xStart;
      for(int i=0; i<=numSlats; i++)
      {
        QGraphicsEllipseItem *p_pole = new QGraphicsEllipseItem(x - kFenceWidth/2.0, yStart - kFenceWidth/2.0,
                                                                kFenceWidth, kFenceWidth);
        p_pole->setBrush(gFencePoleBrush);
        p_pole->setPen(gFencePolePen);
        p_pole->setParentItem(this);
        m_poleList.append(p_pole);

        x += xStep;
      }

      m_boundingRect = QRectF(m_topLeftPos.x(), m_topLeftPos.y(), width, kFenceWidth);
    }
  }

  bool vertical()
  {
    return m_vertical;
  }

private:
  bool m_vertical;
  QGraphicsRectItem m_slatRect;
  QList<QGraphicsEllipseItem *> m_poleList;
};

#endif // FENCE_HPP
