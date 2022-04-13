#ifndef EDITOR_ITEMS_HPP_
#define EDITOR_ITEMS_HPP_

#include "level_types.hpp"

#include <QGraphicsItem>
#include <QtSvg/QGraphicsSvgItem>

class EditorItems : public QGraphicsItem
{
public:
  EditorItems();
  ~EditorItems();

  QRectF boundingRect() const;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

  void init();

  int checkClick(QPointF scenePoint);

private:
  QGraphicsRectItem   m_selectedToolRect;

  QGraphicsRectItem   m_editorLeftBackground;
  QGraphicsRectItem   m_editorRightBackground;

  QGraphicsRectItem   m_editRectItem[NumEditTypes];
  QRectF              m_editRect[NumEditTypes];
  QGraphicsSvgItem    *p_editTools[NumEditTypes];


  qreal m_toolImageWidth;

  bool m_initialized;
};

#endif /* EDITOR_ITEMS_HPP_ */
