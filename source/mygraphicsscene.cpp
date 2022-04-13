#include "mygraphicsscene.hpp"

#include "level.hpp"
#include "global_lists.hpp"
#include "utils.hpp"

#include <cmath>

const qreal kDragThresholdDist = 20.0;

// Helper function to get scene's view () easily
static MyGraphicsView* getView(QGraphicsScene* pScene)
{
  return ((MyGraphicsView*)(pScene->views().at(0)));
}

MyGraphicsScene::MyGraphicsScene()
{
  m_movingDog = false;
  m_draggingDog = false;

  m_zoomTimer = 0;
  m_gameTimer = 0;

  m_barkWaveList.clear();

  m_inTimerHandler = false;

  m_drunkMode = false;

  p_viewPixmap = NULL;
}

MyGraphicsScene::~MyGraphicsScene()
{
  removeItem(&m_buttonBox);
}

void MyGraphicsScene::init(void *pLevel)
{
  p_level = pLevel;
  m_buttonBox.setZValue(100.0);
  addItem(&m_buttonBox);
}

void MyGraphicsScene::reset()
{
  m_movingDog = false;
  m_draggingDog = false;

  while(!m_barkWaveList.isEmpty())
  {
    BarkWave *pBarkWave = m_barkWaveList.front();
    m_barkWaveList.pop_front();

    removeItem(&pBarkWave->m_circle);

    delete pBarkWave;
  }
}

void MyGraphicsScene::timerEvent(QTimerEvent *event)
{
  if(m_inTimerHandler)
  {
    qDebug("Already handling timer!");
    return;
  }

  m_inTimerHandler = true;

  if(event->timerId() == m_gameTimer)
  {
    gCurFrameNumber++;

    // 1) Handle bark wave(s)
    foreach(BarkWave *pBarkWave, m_barkWaveList)
    {
      if(!pBarkWave->update())
      {
        m_barkWaveList.removeOne(pBarkWave);
        removeItem(&pBarkWave->m_circle);
        delete pBarkWave;
      }
    }

    // 2) Update controlling stuff first
    ((Level*)p_level)->update();

    // 3) Advance individual items
    for (int i = 0; i < 2; ++i)
    {
      foreach (QGraphicsItem *item, items())
      {
        // Return if timer was stopped (resp. restarted), e.g. due to the last item's advance being
        // a sheep going to exit.
        if(m_gameTimer == 0 || event->timerId() != m_gameTimer)
        {
          m_inTimerHandler = false;
          return;
        }

        item->advance(i);
      }
    }

    // 4) Handle bonus modes
    if(m_drunkMode)
      handleDrunkMode();

    //!!! save movie
    /*
    MyGraphicsView *pView = getView(this);
    QPixmap pixmap(pView->size());
    (dynamic_cast<QWidget*>(pView))->render(&pixmap);
    char filename[50];
    ::snprintf(filename, 50, "screenshots/img_%06ld.png", gCurFrameNumber);
    pixmap.save(filename);
    */
  }
  else if(event->timerId() == m_zoomTimer)
  {
    ((Level*)p_level)->overviewZooming();
  }

  m_inTimerHandler = false;
}

void MyGraphicsScene::updateZoom(qreal zoomFactor)
{
  qreal tmpZoomFactor = zoomFactor;
  if(tmpZoomFactor > kMaxViewZoomFactor)
    tmpZoomFactor = kMaxViewZoomFactor;
  else if(tmpZoomFactor < kMinViewZoomFactor)
    tmpZoomFactor = kMinViewZoomFactor;

  // Do nothing if minimum or maximum zoom has already been reached before.
  if(gLevelZoomFactor == tmpZoomFactor)
    return;

  gLevelZoomFactor = tmpZoomFactor;
  qDebug("new zoom factor: %2.1f", gLevelZoomFactor);

  ((Level*)p_level)->setScale(gLevelZoomFactor);

  if(curGameMode() == LevelEditMode)
    ((Level*)p_level)->centerLevelInView();
  else
    ((Level*)p_level)->centerDogInView();
}

QPointF MyGraphicsScene::getLevelPos(QPointF scenePos)
{
  return (scenePos - ((Level*)p_level)->scenePos()) / gLevelZoomFactor;
}

void MyGraphicsScene::keyPressEvent(QKeyEvent *p_event)
{

  if(gPinchZooming)
    return;

  Dog *pDog = (Dog*)gDogList.at(0);

  int keyCode = p_event->key();

  //qDebug("key 0x%x pressed", keyCode);

  if(keyCode == gKeyCodeUp1 || keyCode == gKeyCodeUp2)
  {
    pDog->changeAngle(90);
    pDog->changeSpeed(gCurDogSpeed);
  }
  else if(keyCode == gKeyCodeRight1 || keyCode == gKeyCodeRight2)
  {
    pDog->changeAngle(0);
    pDog->changeSpeed(gCurDogSpeed);
  }
  else if(keyCode == gKeyCodeDown1 || keyCode == gKeyCodeDown2)
  {
    pDog->changeAngle(270);
    pDog->changeSpeed(gCurDogSpeed);
  }
  else if(keyCode == gKeyCodeLeft1 || keyCode == gKeyCodeLeft2)
  {
    pDog->changeAngle(180);
    pDog->changeSpeed(gCurDogSpeed);
  }
  else if(keyCode == gKeyCodeBark1 || keyCode == gKeyCodeBark2)
  {
    dogBark(pDog);
  }
}

void MyGraphicsScene::keyReleaseEvent(QKeyEvent *p_event)
{
  if(gPinchZooming)
    return;

  Dog *pDog = (Dog*)gDogList.at(0);

  int keyCode = p_event->key();

#ifdef DEBUG
  qDebug("key '%d' pressed", keyCode);
  trace("Key released: " + " (character code: " + p_event->charCode + ")");
#endif /* DEBUG */

  if(keyCode == gKeyCodeUp1 || keyCode == gKeyCodeUp2)
    pDog->zeroSpeedY();
  else if(keyCode == gKeyCodeRight1 || keyCode == gKeyCodeRight2)
    pDog->zeroSpeedX();
  else if(keyCode == gKeyCodeDown1 || keyCode == gKeyCodeDown2)
    pDog->zeroSpeedY();
  else if(keyCode == gKeyCodeLeft1 || keyCode == gKeyCodeLeft2)
    pDog->zeroSpeedX();
}

void MyGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
  Dog *pDog;
  bool button_clicked = false;

  if(mouseEvent->button() != Qt::LeftButton)
  {
    qDebug("pressed other mouse button than left!");
    mouseEvent->accept();
    return;
  }

  QPointF scenePos = mouseEvent->scenePos();
  mouseEvent->accept();
  m_clickPoint = scenePos;

  if(gShowingMsgBox)
  {
    m_buttonBox.checkHit(m_clickPoint);
    return;
  }

  if(curGameMode() == LevelEditMode)
  {
    ((Level*)p_level)->editClick(m_clickPoint, getLevelPos(m_clickPoint));
    return;
  }

  if(curGameMode() == BuiltInChooserMode || curGameMode() == UserChooserMode || curGameMode() == EditChooserMode)
  {
    ((Level*)p_level)->chooserClick(m_clickPoint);
    return;
  }

  if(curGameMode() == PausedMode)
  {
    //!!! hide inactive background immediately on button push

    if(gRetryButtonRect.contains(scenePos))
    {
      ((Level*)p_level)->resetCurrentLevel();
      gModeList.pop_back();
      goToGameMode(curGameMode());
    }
    else if(gPlayButtonRect.contains(scenePos))
    {
      gModeList.pop_back();
      resumeGame();
    }
    else if(gStopButtonRect.contains(scenePos))
    {
      restorePrevGameMode(false, true);
    }

    return;
  }

  if(gDogList.empty())
  {
    qDebug("dog list empty!");
    mouseEvent->accept();
    return;
  }

  pDog = (Dog*)gDogList.at(0);

  //qDebug("mouse pressed at %2.1f, %2.1f", m_clickPoint.x(), m_clickPoint.y());

  // Buttons are deactivated while the game is paused.
  if( (m_clickPoint.y() + gViewToSceneMap.y() > gWindowHeight - gInfoAreaSize) )
  {
    // If we get here, mouse click was inside info area.

    if(gBarkButtonRect.contains(scenePos))
    {
      button_clicked = true;

      dogBark(pDog);
      return;
    }
    else if(gPauseButtonRect.contains(scenePos))
    {
      button_clicked = true;

      pauseGame();
      return;
    }
  }

  if(!button_clicked)
  {
    if(!gPinchZooming)
    {
      // Make the dog bark if the click was near the dog character.
      QPointF levelClickPoint = getLevelPos(m_clickPoint);
      if(distBetweenPoints(pDog->absMidPoint(), levelClickPoint) > kDogBarkClickThreshold)
      {
        m_movingDog = true;

        pDog->m_curTargetPoint = levelClickPoint;
        moveDog();
        return;
      }
      else
      {
        dogBark(pDog);
        return;
      }
    }
  }
}

void MyGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
  if(gShowingMsgBox)
    return;
  if(curGameMode() == BuiltInChooserMode || curGameMode() == UserChooserMode)
    return;

  if(curGameMode() == LevelEditMode)
  {
    // TODO: why to be divided by 1.1?

    QPointF delta = mouseEvent->scenePos() - mouseEvent->lastScenePos();
    ((Level*)p_level)->editMove(getLevelPos(mouseEvent->scenePos()), delta);

    goto finish;
  }

  if(m_movingDog && distBetweenPoints(m_clickPoint, mouseEvent->scenePos()) > kDragThresholdDist)
  {
    Dog *pDog = (Dog*)gDogList.at(0);
    m_draggingDog = true;

    pDog->m_curTargetPoint = getLevelPos(mouseEvent->scenePos());
    moveDog();
  }

finish:
  mouseEvent->accept();
}

void MyGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
  if(mouseEvent->button() != Qt::LeftButton)
  {
    mouseEvent->accept();
    return;
  }

  mouseEvent->accept();

  if(gShowingMsgBox || curGameMode() == BuiltInChooserMode || curGameMode() == UserChooserMode)
    return;

  if(curGameMode() == LevelEditMode)
  {
    ((Level*)p_level)->editRelease();
    return;
  }

  m_movingDog = false;

  // if dog was moved in "drag mode" before, stop on release (and don't proceed to the last mouse position)
  if(m_draggingDog)
    gDogList.at(0)->changeSpeed(0.0);

  m_draggingDog = false;
}

// Zoom using mouse wheel
void MyGraphicsScene::wheelEvent(QGraphicsSceneWheelEvent *wheelEvent)
{
  updateZoom(gLevelZoomFactor + (qreal)(wheelEvent->delta()) / 600.0);

  wheelEvent->accept();
}

#ifndef DEBUG_OUTPUT
void MyGraphicsScene::focusOutEvent(QFocusEvent *pFocusEvent)
{
  qDebug("Scene lost focus");

  pauseGame();
}
#endif

void MyGraphicsScene::pauseGame(bool showRetryButton)
{
  if(m_gameTimer == 0)
    return;

  killTimer(m_gameTimer);
  m_gameTimer = 0;

  if(showRetryButton)
  {
    addGameMode(PausedMode);
    showPausedScreen(true);
  }
}

void MyGraphicsScene::resumeGame()
{
  if(m_gameTimer != 0)
  {
    qDebug("Timer is already running!");
    return;
  }

  m_gameTimer = startTimer(1000 / kFrameRate);

  gInactiveBackground->setVisible(false);

  gStopButtonItem->setVisible(false);
  gRetryButtonItem->setVisible(false);
  gPlayButtonItem->setVisible(false);
}

#ifdef ANDROID_MOBILE_VERSION
void MyGraphicsScene::closeApp(QCloseEvent *pEvent)
{
  pEvent->accept();
  restorePrevGameMode(true);
}
#endif /* ANDROID_MOBILE_VERSION */

void MyGraphicsScene::moveDog()
{
  Dog *pDog = (Dog*)gDogList.at(0);

  qreal angle = QLineF(pDog->absMidPoint(), pDog->m_curTargetPoint).angle();

  pDog->setSpeedAndAngle(gCurDogSpeed, angle);
}

void MyGraphicsScene::dogBark(Dog *pDog)
{
  if(pDog->bark())
  {
    // add new bark wave if barking was successful:)
    BarkWave *barkWave = new BarkWave(pDog->absMidPoint(), pDog->getCollisionRect()->width() / 2, gMaxBarkDist - (kBarkWaveWidth / 2));
    m_barkWaveList.append(barkWave);
    ((Level*)p_level)->addItemToLevel(&barkWave->m_circle);
  }
}

void MyGraphicsScene::drunkMode(bool activate)
{
  m_drunkMode = activate;

  if(!m_drunkMode)
  {
    removeItem(p_viewPixmap);
    delete p_viewPixmap;
    p_viewPixmap = NULL;
  }
}

// Adds a transparent copy of the view to the scene
void MyGraphicsScene::handleDrunkMode()
{
  MyGraphicsView *pView = getView(this);

  QPixmap pixmap(pView->size());

  // Remove copy before generating a new one to avoid further duplicates of the scene.
  if(p_viewPixmap)
    removeItem(p_viewPixmap);
  ((QWidget*)pView)->render(&pixmap);
  if(p_viewPixmap == NULL)
    p_viewPixmap = new QGraphicsPixmapItem();
  addItem(p_viewPixmap);

  p_viewPixmap->setPixmap(pixmap);
  p_viewPixmap->setOpacity(0.5);
  p_viewPixmap->setPos(gSceneToViewOffset.x() + 12.0 * cos((qreal)gCurFrameNumber / 10.0),
                       gSceneToViewOffset.y() + 5.0 * cos((qreal)gCurFrameNumber / 14.0));
}

void MyGraphicsScene::showMessageBox(const QString &text, const QList<QString> *pButtonTextList,
                                     void (*pButtonHandler)(ButtonContext*), void *pContext)
{
  if(m_buttonBox.init(text, pButtonTextList, pButtonHandler, pContext))
  {
    m_buttonBox.setPos(gSceneToViewOffset.x() + (gWindowWidth - m_buttonBox.width()) / 2.0,
                       gSceneToViewOffset.y() + (gWindowHeight - m_buttonBox.height()) / 2.0);
  }
}

bool MyGraphicsScene::startZooming()
{
  if(m_zoomTimer == 0)
  {
    m_zoomTimer = startTimer(1000 / kFrameRate);
    return true;
  }

  return false;
}

bool MyGraphicsScene::stopZooming()
{
  if(m_zoomTimer != 0)
  {
    killTimer(m_zoomTimer);
    m_zoomTimer = 0;
    return true;
  }

  return false;
}
