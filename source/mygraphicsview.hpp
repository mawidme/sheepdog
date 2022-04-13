#ifndef MYGRAPHICSVIEW_HPP
#define MYGRAPHICSVIEW_HPP

#include "mygraphicsscene.hpp"

#include <QGraphicsView>

#include <QPinchGesture>
#include <QPanGesture>

class MyGraphicsView : public QGraphicsView
{
public:
  MyGraphicsView(MyGraphicsScene *pScene);

protected:
  void closeEvent(QCloseEvent *pEvent);

  bool event(QEvent *pEvent);

private:
  bool gestureEvent(QGestureEvent *pEvent);
  void panTriggered(QPanGesture *gesture);
  void pinchTriggered(QPinchGesture *gesture);

  float m_currentStepScaleFactor;

  MyGraphicsScene *p_scene;
};

#endif // MYGRAPHICSVIEW_HPP
