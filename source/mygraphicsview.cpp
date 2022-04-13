#include "mygraphicsview.hpp"

#include "utils.hpp"
#include "global_lists.hpp"
#include "level.hpp"

MyGraphicsView::MyGraphicsView(MyGraphicsScene *pScene)
{
  p_scene = pScene;

  m_currentStepScaleFactor = 1;

  //!!! doesn't work correctly yet
  //grabGesture(Qt::PinchGesture);
}

void MyGraphicsView::closeEvent(QCloseEvent *pEvent)
{
#ifdef ANDROID_MOBILE_VERSION
  if(p_scene == NULL)
  {
    qDebug("cannot forward close event as scene pointer is null!");
  }
  else
  {
    qDebug("forwarding close event to scene...");

    p_scene->closeApp(pEvent);
  }
#endif
}

bool MyGraphicsView::event(QEvent *pEvent)
{
  if (pEvent->type() == QEvent::Gesture)
  {
    QGesture *pinch = ((QGestureEvent*)pEvent)->gesture(Qt::PinchGesture);
    if(pinch != NULL)
    {
      pinchTriggered(static_cast<QPinchGesture *>(pinch));
      return true;
    }
  }

  return QGraphicsView::event(pEvent);
}

void MyGraphicsView::pinchTriggered(QPinchGesture *gesture)
{
  if(gesture->changeFlags() & QPinchGesture::ScaleFactorChanged)
  {
    gPinchZooming = true;

    qreal value = gesture->property("scaleFactor").toReal();
    m_currentStepScaleFactor *= value;

    //!!! too slow!
    //p_scene->updateZoom(value);
  }

  if (gesture->state() == Qt::GestureFinished)
  {
    gPinchZooming = false;

    p_scene->updateZoom(m_currentStepScaleFactor);

    m_currentStepScaleFactor = 1;

    qDebug("finished pinch scale gesture: scale factor = %3.1f", gLevelZoomFactor);
  }
}

/*!!! testing
bool MyGraphicsView::event(QEvent *pEvent)
{
  if(pEvent->type() == QEvent::UpdateRequest)
  {
    // do nothing
  }
  else if(pEvent->type() == QEvent::Leave)
    qDebug("Leave");
  else if(pEvent->type() == QEvent::NonClientAreaMouseMove)
    qDebug("NonClientAreaMouseMove");
  else if(pEvent->type() == QEvent::WindowDeactivate)
    qDebug("WindowDeactivate");
  else if(pEvent->type() == QEvent::ActivationChange)
    qDebug("ActivationChange");
  else
    qDebug("event type %d", pEvent->type());

  return QGraphicsScene::event(pEvent);
}
*/
