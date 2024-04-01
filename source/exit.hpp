#ifndef EXIT_HPP
#define EXIT_HPP

#include "level_element.hpp"
#include "level_types.hpp"

#include <QtWidgets/QGraphicsPixmapItem>

class ExitElem : public LevelElement
{
public:
  ExitElem(qreal sx, qreal sy, long unsigned int style)
  {
    m_topLeftPos = QPointF(sx, sy);
    //qDebug("allocating exit 0x%x at (%.0f, %.0f)", (unsigned int)this, sx, sy);

    m_elemType = ExitLevelElement;

		m_layer = 2;

    // add graphics
    m_imgStyle = style - 1;
    m_imgFileName = QString("exit%1").arg(m_imgStyle+1);

    QSizeF imgSize = loadElmtGraphics(sx, sy);

    m_boundingRect = QRectF(sx, sy, imgSize.width(), imgSize.height());
  }
};

#endif // EXIT_HPP
