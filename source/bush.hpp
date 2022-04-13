#ifndef BUSH_HPP
#define BUSH_HPP

#include "level_element.hpp"
#include "level_types.hpp"

class Bush : public LevelElement
{
public:
  Bush(qreal sx, qreal sy, long unsigned int style)
  {
    m_topLeftPos = QPointF(sx, sy);
    //qDebug("allocating bush 0x%x at (%.0f, %.0f)", (unsigned int)this, sx, sy);

    m_elemType = BushLevelElement;

    m_layer = 1;

    // add graphics
    m_imgStyle = style - 1;
    m_imgFileName = QString("bush%1").arg(m_imgStyle+1);

    QSizeF imgSize = loadElmtGraphics(sx, sy);

    initColRect(sx + 8, sy + 8,
                24, 24);

    m_boundingRect = QRectF(sx, sy, imgSize.width(), imgSize.height());
  }
};

#endif // BUSH_HPP
