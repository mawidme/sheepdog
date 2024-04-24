#ifndef DECO_HPP_
#define DECO_HPP_

#include "level.hpp"

enum DecoType
{
  GrassDeco,
  NumDecoTypes
};

const QPen   kGrassBladePen(QColor(0, 42, 0));
const QBrush kGrassBladeBrush(QColor(0, 53, 0));

class Deco : public LevelElement
{
public:
  ~Deco()
  {
    while(!m_lineList.isEmpty())
    {
      QGraphicsLineItem *pLine = m_lineList.front();
      m_lineList.pop_front();
      delete pLine;
    }
  }

  Deco(long unsigned int sx, long unsigned int sy)
  {
    m_topLeftPos = QPointF(sx, sy);
    m_bottomRightPos = QPointF(sx, sy); // fake calculation - currently not needed
    qDebug("allocating deco 0x%llx at (%ld, %ld)", (unsigned long long)this, sx, sy);

    m_elemType = DecoLevelElement;

    m_layer = 0;

    m_imgType = CustomImgType;

    m_boundingRect = QRectF();

    int type = getRandom(1, NumDecoTypes) - 1;

    switch(type)
    {
      case GrassDeco:
        {
          qreal x = 0;
          qreal xd = 0;
          qreal height;
          // random number blades of grass (* 2 = number of lines to draw)
          int num = getRandom(6, 14);
          for(int i=0; i<num; i++)
          {
            QGraphicsLineItem *pLine;

            // random thickness
            xd = getRandom(1, 3);

            if(i % 2 == 0)
            {
              height = -getRandom(7, 12);
              // the first and the last blade are shorter
              if( (i == 0) || (i == num-2) )
                height *= 0.6;
              pLine = new QGraphicsLineItem(x, 0.0, x+xd, height);
            }
            else
            {
              pLine = new QGraphicsLineItem(x, height, x+xd, 0);
            }

            pLine->setPen(kGrassBladePen);
            pLine->setParentItem(this);
            m_lineList.append(pLine);

            x += xd;
          }
        }
        break;

      default:
        qDebug("invalid deco type!");
        return;
    }
  }

private:
  QList<QGraphicsLineItem *> m_lineList;
};

#endif /* DECO_HPP_ */
