#ifndef BUTTON_BOX_HPP_
#define BUTTON_BOX_HPP_

#include "globals.hpp"
#include "utils.hpp"

#include <QGraphicsItem>
#include <QtSvg/QGraphicsSvgItem>

struct ButtonContext
{
  int buttonNumber;
  void (*p_handlerFunction)(ButtonContext*);
  void *p_context;
};


class MyButton : public QGraphicsItem
{
public:
  void init(const QString &text);
  qreal width();
  qreal height();
  bool contains(const QPointF &point) const;
  void clear();

  QRectF boundingRect() const;
  // Dummy implementation of virtual QGraphicsItem methods.
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {};

private:
  QGraphicsTextItem m_buttonTextItem;
  QGraphicsRectItem m_buttonFrameItem;
};


class MyButtonBox : public QGraphicsItem
{
public:
  MyButtonBox();

  bool init(const QString &text, const QList<QString> *pButtonTextList,
            void (*pButtonHandler)(ButtonContext*), void *pContext);

  bool checkHit(const QPointF &sceneClickPos);

  void clear();

  qreal width();
  qreal height();

  QRectF boundingRect() const;
  // Dummy implementation of virtual QGraphicsItem methods.
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {};

private:
  bool m_initialized;

  // Callback context for button press
  ButtonContext m_buttonContext;

  QList<MyButton*> m_buttonList;

  QGraphicsTextItem m_boxTextItem;
  QGraphicsSvgItem *p_boxFrameItem;
};

#endif /* BUTTON_BOX_HPP_ */
