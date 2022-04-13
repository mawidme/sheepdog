#ifndef TREE_HPP
#define TREE_HPP

#include "level_element.hpp"
#include "level_types.hpp"

class Tree : public LevelElement
{
public:
  Tree(qreal sx, qreal sy, long unsigned int style)
  {
    m_topLeftPos = QPointF(sx, sy);
    //qDebug("allocating tree 0x%x at (%.0f, %.0f)", (unsigned int)this, sx, sy);

    m_elemType = TreeLevelElement;

    m_layer = 3;

    // add graphics
    m_imgStyle = style - 1;
    m_imgFileName = QString("tree%1").arg(m_imgStyle+1);

    QSizeF imgSize = loadElmtGraphics(sx, sy);

    initColRect(sx + 28, sy + 28,
                15, 15);

    m_boundingRect = QRectF(sx, sy, imgSize.width(), imgSize.height());
  }
};

#endif // TREE_HPP
