#include "level.hpp"

#include "xml.hpp"

#include "deco.hpp"

#include "utils.hpp"

#include "global_lists.hpp"

// include level element headers for editing
#include "fence.hpp"
#include "bush.hpp"
#include "tree.hpp"
#include "exit.hpp"
#include "water.hpp"

#include <QtWidgets/QMessageBox>

#if defined(USE_OPENGL) && !defined(QT_NO_OPENGL)
#include <QtOpenGL/QGLWidget>
#include <QtOpenGL/QGLFormat>
#endif

// TODO: calculate this dynamically for larger levels!
static const qreal kOverviewStartZoom = 0.3;

//!!! make zooming faster for mobile device! (use picture of level instead of real level elements etc.)
#ifdef MOBILE_VERSION
static const qreal kOverviewZoomDuration = 0.5; // seconds
#else
static const qreal kOverviewZoomDuration = 2.0; // seconds
#endif

QPointF kMinPointInit(10000, 10000);
QPointF kMaxPointInit(0, 0);
QPointF kZeroPoint(0, 0);

Level::Level() : p_scene(NULL),
                 p_view(NULL),
                 m_guiElementsAdded(false),
                 m_viewInitDone(false),
                 p_bonus(NULL),
                 m_limitsInitialized(false),
                 m_askForSave(false),
                 m_rescanLevels(false)
{
  m_width = 0;
  m_height = 0;

  m_numTotalSheep = 0;
  m_numRemainingSheep = 0;

  m_timeLeft = 0;
  m_totalTime = 0;
  m_bonusTime = 0;
}

Level::~Level()
{
  gLevel = NULL;

  if(p_scene != NULL)
    p_scene->removeItem(this);

  clear();
  removeGuiElements();

  // bonus should be freed before scene as its destructor removes
  if(p_bonus != NULL)
  {
    delete p_bonus;
    p_bonus = NULL;
  }

  if(p_scene != NULL)
  {
    delete p_scene;
    p_scene = NULL;
  }

  if(p_view != NULL)
    delete p_view;
}

// When this is called from destructor, items don't have to be removed from the scene
// anymore as this is done by the QGraphicsScene destructor.
void Level::clear()
{
  m_numTotalSheep = 0;
  m_numRemainingSheep = 0;

  m_width = 0;
  m_height = 0;

  if(p_scene != NULL)
  {
    removeGameObjects();

    // reset scene's inside parameters
    p_scene->reset();

    qDebug("number of level elements after reset: %d", p_scene->items().size());
  }
}

void Level::removeGameObjects()
{
  qDebug("removing %d level element(s)...", gLevelElementList.size());
  while(!gLevelElementList.isEmpty())
  {
    LevelElement *p_element = gLevelElementList.front();
    gLevelElementList.pop_front();
    delete p_element;
  }

  qDebug("removing %d dog(s)...", gDogList.size());
  while(!gDogList.isEmpty())
  {
    Dog *p_dog = (Dog*)(gDogList.front());
    gDogList.pop_front();
    delete p_dog;
  }

  qDebug("removing %d sheep...", gSheepList.size());
  while(!gSheepList.isEmpty())
  {
    Sheep *p_sheep = (Sheep*)(gSheepList.front());
    gSheepList.pop_front();
    delete p_sheep;
  }
}

void Level::setupSceneAndView()
{
  gViewToSceneMap = QPointF(0, 0);

  if(m_viewInitDone)
    return;

  //-------------------------------------------------------------------------------------------------------
  // GAME AREA
  if(p_scene == NULL)
  {
    p_scene = new MyGraphicsScene();
    qDebug("scene created");
  }

  p_scene->init(this);

  // TODO: make scene size dynamic (e.g. level size + 100 pixel)

  p_scene->setSceneRect(0, 0, kSceneWidth, kSceneHeight);
  p_scene->setItemIndexMethod(QGraphicsScene::NoIndex);
  p_scene->setFocus(Qt::ActiveWindowFocusReason);

  if(p_view == NULL)
  {
    p_view = new MyGraphicsView(p_scene);
    qDebug("view created");
  }

  p_view->setScene(p_scene);

  p_view->setBackgroundBrush(QBrush(gBackgroundColor));

  p_view->setCacheMode(QGraphicsView::CacheBackground);
#if defined(USE_OPENGL) && !defined(QT_NO_OPENGL)
  p_view->setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
#endif
  p_view->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
  p_view->setDragMode(QGraphicsView::NoDrag);
  p_view->setWindowTitle(QT_TRANSLATE_NOOP(QGraphicsView, "sheep dog"));
  p_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  p_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  p_view->resize(gWindowWidth, gWindowHeight);
  p_view->setSceneRect((p_scene->width() - gWindowWidth) / 2,
                       (p_scene->height() - gWindowHeight) / 2,
                       gWindowWidth, gWindowHeight);

  if(gGraphicQuality >= HighQuality)
    p_view->setRenderHint(QPainter::Antialiasing);
  else if(gGraphicQuality >= MedQuality)
    p_view->setRenderHint(QPainter::SmoothPixmapTransform);
  else
    p_view->setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing);

#ifdef MOBILE_VERSION
  p_view->showFullScreen();
#else
  p_view->show();
#endif

  // add the level itself to the scene
  p_scene->addItem(this);
  addGuiElements();

  m_viewInitDone = true;
}

void Level::menu()
{
  m_levelNumber = 0;
  if(!loadLevel(BuiltInSource))
    exit(EXIT_FAILURE);

  setVisible(true);
  m_infoItems.setVisible(false);
  m_editorItems.setVisible(false);
  m_levelChooseItems.setVisible(false);

  QList<QString> stringList;
  stringList.append(QString("play"));
  stringList.append(QString("play own levels"));
  stringList.append(QString("editor"));
  p_scene->showMessageBox(QString(), &stringList, mainMenuClickedStep1, this);
}

void Level::mainMenuClickedStep1(ButtonContext *pButtonContext)
{
  ((Level*)pButtonContext->p_context)->mainMenuClickedStep2(pButtonContext->buttonNumber);
}

void Level::mainMenuClickedStep2(int button)
{
  switch(button)
  {
    case 1:
      goToGameMode(BuiltInChooserMode);
      break;

    case 2:
      goToGameMode(UserChooserMode);
      break;

    case 3:
      initEditor();
      break;

    default:
      break;
  }
}

void Level::initEditor()
{
  m_levelNumber = 0;
  removeGameObjects();

  m_maxPoint = kMaxPointInit;

  // set this to false first because nothing has been added yet
  // (and therefore it doesn't matter if we leave the currently edited level)
  m_askForSave = false;

  m_editorItems.init();

  goToGameMode(LevelEditMode);
}

void Level::editor()
{
  setVisible(true);
  m_infoItems.setVisible(false);
  m_editorItems.setVisible(true);
  m_levelChooseItems.setVisible(false);

  if(!m_limitsInitialized)
  {
    // just set some initial level size
    m_width = 200.0;
    m_height = 200.0;
  }

  // center whole level in view
  centerLevelInView();
}

void Level::editClick(QPointF scenePoint, QPointF levelPoint)
{
  // TODO: add style switch (e.g. bush, tree)

  // check if edit tool button was clicked or not:
  int type = m_editorItems.checkClick(scenePoint);
  if(type == -1)
  {
    m_dragEdit = true;
    editClickLevel(levelPoint);
  }
  else
  {
    editClickButton(type);
  }
}

void Level::editClickButton(int type)
{
  if(type == (int)UndoEdit)
  {
    if(!m_undoList.empty())
    {
      switch(m_undoList.back())
      {
        case ElmtObj:
          {
            if(!gLevelElementList.empty())
            {
              LevelElement *pLevelElmt = gLevelElementList.back();
              gLevelElementList.pop_back();
              delete pLevelElmt;
            }
          }
          break;
        case DogObj:
          {
            if(!gDogList.empty())
            {
              Character *pDog = gDogList.back();
              gDogList.pop_back();
              delete pDog;
            }
          }
          break;
        case SheepObj:
          {
            if(!gSheepList.empty())
            {
              Character *pSheep = gSheepList.back();
              gSheepList.pop_back();
              delete pSheep;
            }
          }
          break;
      }

      m_undoList.pop_back();

      qDebug("undo: removed from undo list");

      editCheckEmpty();
    }
  }
  else if(type == (int)PlayEdit)
  {
    int err = checkLevel();
    if(err != NoError)
      return;

    editPrepareLevel();

    goToGameMode(PlayMode);
  }
  else if(type == (int)LoadEdit)
  {
    if(m_askForSave)
    {
      gNextMode = EditChooserMode;
      askForSave();
      return;
    }

    goToGameMode(EditChooserMode);
  }
  else if(type == (int)SaveEdit)
  {
    saveLevel();
  }
  else if(type == (int)LeaveEdit)
  {
    leaveEditor();
  }
  else if(type >= NumEditTypes)
  {
    qDebug("invalid edit tool button type %d!", type);
  }
  else
  {
    // nothing to do for the remaining buttons (mode is already changed earlier according to the button pressed)
  }
}

void Level::editClickLevel(QPointF levelPoint)
{
  m_editClickPoint = levelPoint;

  if(m_editClickPoint.x() < 0.0 || m_editClickPoint.y() < 0.0)
  {
    if(gEditMode == FenceEdit ||
       gEditMode == BushEdit ||
       gEditMode == TreeEdit ||
       gEditMode == ExitEdit ||
       gEditMode == WaterEdit ||
       gEditMode == DogEdit ||
       gEditMode == SheepEdit)
    {
      qreal safetyDist = 50.0;
      QPointF delta(0, 0);
      if(m_editClickPoint.x() < 0.0)
        delta.setX(safetyDist - m_editClickPoint.x());
      if(m_editClickPoint.y() < 0.0)
        delta.setY(safetyDist - m_editClickPoint.y());
      moveLevelBy(delta);
      m_editClickPoint += delta;
    }
  }

  QSizeF imgSize;
  switch(gEditMode)
  {
    case FenceEdit:
      if(editAddFence())
        m_askForSave = true;
      break;
    case BushEdit:
    case TreeEdit:
    case ExitEdit:
      if(editAddImgObj())
        m_askForSave = true;
      break;
    case WaterEdit:
      // nothing to do yet - won't be drawn until big enough
      break;
    case DogEdit:
      {
        //!!! add collision check!

        m_askForSave = true;
        m_undoList.append(DogObj);

        // currently, only one dog is allowed - delete old one if a second is added.
        while(!gDogList.empty())
        {
          Character *pDog = gDogList.front();
          gDogList.pop_front();
          delete pDog;
        }

        Dog *dog = new Dog(m_editClickPoint.x(), m_editClickPoint.y());
        gDogList.append(dog);
        addObjToLevel(dog);
      }
      break;
    case SheepEdit:
      {
        //!!! add collision check!

        m_askForSave = true;
        m_undoList.append(SheepObj);

        Sheep *sheep = new Sheep(m_editClickPoint.x(), m_editClickPoint.y());
        gSheepList.append(sheep);
        addObjToLevel(sheep);
      }
      break;
    case EraserEdit:
      editErase();
      break;
    case UndoEdit:
    case PanEdit:
    case LoadEdit:
    case SaveEdit:
    case PlayEdit:
    case LeaveEdit:
    case NumEditTypes:
      // nothing to do in these modes for mouse click outside of buttons...
      break;
  }
}

void Level::moveLevelBy(QPointF delta)
{
  if(delta.isNull())
    return;

  // also move view to keep objects visible resp. "in place"
  moveBy(-(gLevelZoomFactor * delta.x()), -(gLevelZoomFactor * delta.y()));

  qDebug("moving all game objects by %.0lfx%.0lf...", delta.x(), delta.y());

  foreach(LevelElement *pElmt, gLevelElementList)
    pElmt->moveByDeltaPoint(delta);

  foreach(Character *pSheep, gSheepList)
    pSheep->moveByDeltaPoint(delta);

  foreach(Character *pDog, gDogList)
    pDog->moveByDeltaPoint(delta);

  m_maxPoint += delta;
}

// re-calculates min/max point (e.g. necessary after removing an element from level)
void Level::editSearchLimits()
{
  if(gLevelElementList.empty() &&
     gSheepList.empty() &&
     gDogList.empty())
  {
    m_limitsInitialized = false;
    qDebug("no level objects -> cannot search limits!");
    return;
  }

  m_minPoint = kMinPointInit;
  m_maxPoint = kMaxPointInit;

  foreach(LevelElement *pElmt, gLevelElementList)
  {
    m_minPoint = QPointF(qMin(m_minPoint.x(), pElmt->getTopLeftPos().x()),
                         qMin(m_minPoint.y(), pElmt->getTopLeftPos().y()));
    m_maxPoint = QPointF(qMax(m_maxPoint.x(), pElmt->getBottomRightPos().x()),
                         qMax(m_maxPoint.y(), pElmt->getBottomRightPos().y()));
  }

  foreach(Character *pSheep, gSheepList)
  {
    m_minPoint = QPointF(qMin(m_minPoint.x(), pSheep->getTopLeftPos().x()),
                         qMin(m_minPoint.y(), pSheep->getTopLeftPos().y()));
    m_maxPoint = QPointF(qMax(m_maxPoint.x(), pSheep->getBottomRightPos().x()),
                         qMax(m_maxPoint.y(), pSheep->getBottomRightPos().y()));
  }

  foreach(Character *pDog, gDogList)
  {
    m_minPoint = QPointF(qMin(m_minPoint.x(), pDog->getTopLeftPos().x()),
                         qMin(m_minPoint.y(), pDog->getTopLeftPos().y()));
    m_maxPoint = QPointF(qMax(m_maxPoint.x(), pDog->getBottomRightPos().x()),
                         qMax(m_maxPoint.y(), pDog->getBottomRightPos().y()));
  }

  m_limitsInitialized = true;
}

bool Level::adjustLimits()
{
  if(m_limitsInitialized)
  {
    m_width = m_maxPoint.x() - m_minPoint.x();
    m_height = m_maxPoint.y() - m_minPoint.y();
    // level bounding rectangle change has to be announced:
    prepareGeometryChange();
    return true;
  }
  else
  {
    qDebug("limits not set -> will not set dimensions");
    return false;
  }
}

void Level::editCheckEmpty()
{
  // if all lists are empty after this, leaving the current level editing session doesn't have to be avoided.
  if(gLevelElementList.empty() && gDogList.empty() && gSheepList.empty())
  {
    m_askForSave = false;
    m_maxPoint = kMaxPointInit;
    m_limitsInitialized = false;
  }
  else
  {
    m_askForSave = true;
    editSearchLimits();
  }
}

void Level::editPrepareLevel()
{
  // TODO: change level time in editor
  m_totalTime = 120;
  m_timeLeft = m_totalTime;
  m_bonusTime = 0;

  if(!m_minPoint.isNull())
    moveLevelBy(-m_minPoint);

  adjustLimits();
}

bool Level::editAddFence()
{
  m_undoList.append(ElmtObj);

  //!!! check for character collisions

  bool foundNearFence = false;
  qreal searchDist = 20.0;
  QRectF fenceSearchRect(m_editClickPoint - QPointF(searchDist, searchDist),
                         m_editClickPoint + QPointF(searchDist, searchDist));
  QPointF adjustedStartPoint = m_editClickPoint;
  foreach(LevelElement *pElmt, gLevelElementList)
  {
    if(pElmt->getElemType() == FenceLevelElement &&
       fenceSearchRect.intersects(pElmt->boundingRect()))
    {
      // first check if clicked point is close to the start or end point of the found near fence.
      // if that is the case, save the corresponding point: 1.a) / 1.b)
      // otherwise, only adjust one coordinate component to connect to fence: 2.)
      Fence *pFence = ((Fence*)pElmt);

      qreal endDist = distBetweenPoints(pFence->getBottomRightPos() - QPointF(kFenceWidth / 2.0, kFenceWidth / 2.0),
                                        m_editClickPoint);
      qDebug("dist to end point: %lf", endDist);

      if(distBetweenPoints(pFence->getTopLeftPos() + QPointF(kFenceWidth / 2.0, kFenceWidth / 2.0),
                           m_editClickPoint) < searchDist)
      {
        // 1.a)
        adjustedStartPoint = pFence->getTopLeftPos() + QPointF(kFenceWidth / 2.0, kFenceWidth / 2.0);
      }
      else if(distBetweenPoints(pFence->getBottomRightPos() - QPointF(kFenceWidth / 2.0, kFenceWidth / 2.0),
                                m_editClickPoint) < searchDist)
      {
        // 1.b)
        adjustedStartPoint = pFence->getBottomRightPos() - QPointF(kFenceWidth / 2.0, kFenceWidth / 2.0);
      }
      else
      {
        // 2.)
        if(pFence->vertical())
          adjustedStartPoint.setX(pElmt->getTopLeftPos().x() + kFenceWidth / 2.0);
        else
          adjustedStartPoint.setY(pElmt->getTopLeftPos().y() + kFenceWidth / 2.0);
      }

      m_editClickPoint = adjustedStartPoint;

      foundNearFence = true;
      break;
    }
  }

  Fence *fence;
  if(foundNearFence)
  {
    fence = new Fence(adjustedStartPoint.x(), adjustedStartPoint.y(),
                      adjustedStartPoint.x(), adjustedStartPoint.y());
  }
  else
  {

    fence = new Fence(m_editClickPoint.x(), m_editClickPoint.y(),
                      m_editClickPoint.x(), m_editClickPoint.y());
  }
  gLevelElementList.append(fence);
  addObjToLevel(fence);

  return true;
}

bool Level::editAddWater(QPointF levelPoint)
{
  if(fabs(m_editClickPoint.x() - levelPoint.x()) < kMinWaterSize ||
     fabs(m_editClickPoint.y() - levelPoint.y()) < kMinWaterSize)
    return false;

  Water *water = new Water(m_editClickPoint, levelPoint, 1);
  gLevelElementList.append(water);
  addObjToLevel(water);

  m_undoList.append(ElmtObj);

  return true;
}

bool Level::editAddImgObj()
{
  //!!! check for character collisions

  m_undoList.append(ElmtObj);

  LevelElement *pElmt;
  if(gEditMode == BushEdit)
  {
    Bush *bush = new Bush(m_editClickPoint.x(), m_editClickPoint.y(), 1);
    pElmt = bush;
  }
  else if(gEditMode == TreeEdit)
  {
    Tree *tree = new Tree(m_editClickPoint.x(), m_editClickPoint.y(), 1);
    pElmt = tree;
  }
  else if(gEditMode == ExitEdit)
  {
    ExitElem *exitElem = new ExitElem(m_editClickPoint.x(), m_editClickPoint.y(), 1);
    pElmt = exitElem;
  }
  else
  {
    qDebug("invalid edit mode: %d", gEditMode);
    return false;
  }

  pElmt->moveByDelta(-pElmt->boundingRect().width() / 2.0,
                     -pElmt->boundingRect().height() / 2.0);
  gLevelElementList.append(pElmt);
  addObjToLevel(pElmt);

  return true;
}

void Level::editErase()
{
  qreal searchDist = 10.0;
  QRectF searchRect(m_editClickPoint - QPointF(searchDist, searchDist),
                    m_editClickPoint + QPointF(searchDist, searchDist));

  foreach(LevelElement *pElmt, gLevelElementList)
  {
    if(searchRect.intersects(*pElmt->getCollisionRect()))
    {
      // also delete corresponding entry from undo list!
      int index = gLevelElementList.indexOf(pElmt);
      if(index == -1)
      {
        qDebug("could not get element index!");
      }
      else
      {
        if(removeItemFromUndoList(ElmtObj, index+1))
          qDebug("erase: removed element from undo list");
      }

      gLevelElementList.removeOne(pElmt);
      delete pElmt;

      goto erased;
    }
  }

  foreach(Character *pDog, gDogList)
  {
    if(searchRect.intersects(*pDog->getCollisionRect()))
    {
      // also delete corresponding entry from undo list!
      int index = gDogList.indexOf(pDog);
      if(index == -1)
      {
        qDebug("could not get dog index!");
      }
      else
      {
        if(removeItemFromUndoList(DogObj, index+1))
          qDebug("erase: removed dog from undo list");
      }

      gDogList.removeOne(pDog);
      delete pDog;

      goto erased;
    }
  }

  foreach(Character *pSheep, gSheepList)
  {
    if(searchRect.intersects(*pSheep->getCollisionRect()))
    {
      // also delete corresponding entry from undo list!
      int index = gSheepList.indexOf(pSheep);
      if(index == -1)
      {
        qDebug("could not get sheep index!");
      }
      else
      {
        if(removeItemFromUndoList(SheepObj, index+1))
          qDebug("erase: removed sheep from undo list");
      }

      gSheepList.removeOne(pSheep);
      delete pSheep;

      goto erased;
    }
  }

  return;

erased:
  editCheckEmpty();
}

bool Level::removeItemFromUndoList(GameObjType type, int num)
{
  if(!m_undoList.empty())
  {
    int numFound = 0;
    for(int undoIndex=0; undoIndex<m_undoList.size(); undoIndex++)
    {
      GameObjType obj = m_undoList.at(undoIndex);
      if(obj == type)
      {
        numFound++;
        if(numFound == num)
        {
          m_undoList.removeAt(undoIndex);
          return true;
        }
      }
    }
  }

  return false;
}

int Level::checkLevel()
{
  if(gDogList.empty())
  {
    QList<QString> stringList;
    stringList.append(QString("Ok"));
    p_scene->showMessageBox(QString("add at least one dog\nto the level!"), &stringList, NULL, NULL);
    return NoDogError;
  }

  if(gSheepList.empty())
  {
    QList<QString> stringList;
    stringList.append(QString("Ok"));
    p_scene->showMessageBox(QString("add at least one sheep\nto the level!"), &stringList, NULL, NULL);
    return NoSheepError;
  }

  bool exitFound = false;
  foreach(LevelElement *pElement, gLevelElementList)
  {
    if(pElement->getElemType() == ExitLevelElement)
      exitFound = true;
  }

  if(!exitFound)
  {
    QList<QString> stringList;
    stringList.append(QString("Ok"));
    p_scene->showMessageBox(QString("add at least one exit\nto the level!"), &stringList, NULL, NULL);
    return NoExitError;
  }

  return NoError;
}

int Level::saveLevel()
{
  int err = checkLevel();
  if(err != NoError)
    return err;

  editPrepareLevel();

  // save button clicked
  Xml xml;
  LevelSettings levelSettings;
  levelSettings.m_width = m_width;
  levelSettings.m_height = m_height;
  levelSettings.m_time = m_totalTime;

  err = xml.saveLevelXmlFile(&levelSettings, &m_levelNumber);
  if(err == NoError)
  {
    m_askForSave = false;
    m_rescanLevels = true;
  }
  else
  {
    return err;
  }

  centerLevelInView();

  return NoError;
}

void Level::editMove(QPointF levelPoint, QPointF delta)
{
  // there's nothing to do if mouse button is not down (drag mode)
  if(!m_dragEdit)
    return;

  //!!! add scrolling for longer fences / water pools

  switch(gEditMode)
  {
    case FenceEdit:
      {
        //!!! snap end point to existing fence coordinates!

        LevelElement *delFence = gLevelElementList.back();
        gLevelElementList.pop_back();
        delete delFence;

        Fence *fence = new Fence(m_editClickPoint.x(), m_editClickPoint.y(),
                                 levelPoint.x(), levelPoint.y());
        gLevelElementList.append(fence);
        addObjToLevel(fence);
      }
      break;
    case BushEdit:
      gLevelElementList.back()->moveByDeltaPoint(delta);
      break;
    case TreeEdit:
      gLevelElementList.back()->moveByDeltaPoint(delta);
      break;
    case WaterEdit:
      if(m_drawingItem)
      {
        // existing water has to be removed - it's too small now!
        LevelElement *delWater = gLevelElementList.back();
        gLevelElementList.pop_back();
        delete delWater;
      }

      m_drawingItem = false;

      if(editAddWater(levelPoint))
      {
        m_askForSave = true;
        m_drawingItem = true;
      }
      break;
    case ExitEdit:
      gLevelElementList.back()->moveByDeltaPoint(delta);
      break;
    case DogEdit:
      gDogList.back()->moveByDeltaPoint(delta);
      break;
    case SheepEdit:
      gSheepList.back()->moveByDeltaPoint(delta);
      break;
    case PanEdit:
      {
        QPointF d = delta / 1.1;
        moveBy(d.x(), d.y());
      }
      break;
    case UndoEdit:
    case EraserEdit:
    case LoadEdit:
    case SaveEdit:
    case PlayEdit:
    case LeaveEdit:
    case NumEditTypes:
      // nothing to do in these modes for dragging...
      break;
  }
}

void Level::editRelease()
{
  m_dragEdit = false;
  m_drawingItem = false;
}

void Level::leaveEditor()
{
  if(m_askForSave)
  {
    gNextMode = MainMenuMode;
    askForSave();
    return;
  }

  goToMainMenuMode();
}

void Level::askForSave()
{
  QList<QString> stringList;
  stringList.append(QString("save now"));
  stringList.append(QString("don't save"));
  stringList.append(QString("continue editing"));
  p_scene->showMessageBox(QString("level has not been saved!\n\nwhat do you want to do?"), &stringList, askForSaveClickedStep1, this);
}

void Level::askForSaveClickedStep1(ButtonContext *pButtonContext)
{
  ((Level*)pButtonContext->p_context)->askForSaveClickedStep2(pButtonContext->buttonNumber);
}

void Level::askForSaveClickedStep2(int button)
{
  bool doNextAction = false;

  switch(button)
  {
    case 1:
      {
        int err = saveLevel();

        // there's nothing left to do about errors while creating directories resp. files:
        // they will be shown using a standard qt message box in Xml::saveLevelXmlFile()
        // and we'll continue with whatever action is next.
        if(err == NoError ||
           err == DirCreateError ||
           err == OpenFileError ||
           err == WriteFileError)
          doNextAction = true;
      }
      break;
    case 2:
      doNextAction = true;
      break;
    case 3:
      // nothing to do here - just continue editing
      break;
  }

  if(doNextAction)
    goToGameMode(gNextMode);
}

bool Level::init()
{
  // TODO: message box on return false

  setupSceneAndView();

  if(p_bonus == NULL)
    p_bonus = new Bonus();
  p_bonus->init(this, p_view, p_scene);

  setScale(gLevelZoomFactor);

  return true;
}

bool Level::showChooserDialog(const LevelSource source)
{
  if(!m_levelChooseItems.init(source, m_rescanLevels))
    return false;

  m_rescanLevels = false;

  // do not show level objects in this dialog
  setVisible(false);
  m_infoItems.setVisible(false);
  m_editorItems.setVisible(false);
  m_levelChooseItems.setVisible(true);

  return true;
}

void Level::chooserClick(QPointF scenePoint)
{
  int number = m_levelChooseItems.checkClick(scenePoint);
  if(number == -1)
  {
    // no button hit -> do nothing
  }
  else if(number == 0)
  {
    // exit button hit -> return to previous mode
    restorePrevGameMode(false);
  }
  else
  {
    m_levelNumber = number;

    if(curGameMode() == EditChooserMode)
    {
      if(((Level*)gLevel)->loadLevel(UserSource))
        m_askForSave = false;
      else
        gNextMode = MainMenuMode;
    }

    goToGameMode(gNextMode);
  }
}

bool Level::loadLevel(const LevelSource source)
{
  clear();

  Xml xml;
  LevelSettings levelSettings;
  if(!xml.parse(source, m_levelNumber, &levelSettings))
  {
    QList<QString> stringList;
    if(m_levelNumber > 1)
    {
      stringList.append(QString("ok"));
      p_scene->showMessageBox(QString("all levels done!"), &stringList, allLevelsDoneClickedStep1, this);
    }
    else
    {
      stringList.append(QString("EXIT!"));
      p_scene->showMessageBox(QString("Oooops...error parsing level!"), &stringList, exitErrorClickedStep1, this);
    }

    clear();

    return false;
  }

  m_limitsInitialized = true;
  m_minPoint = QPointF(0, 0);
  m_maxPoint = QPointF(levelSettings.m_width, levelSettings.m_height);
  adjustLimits();

  m_totalTime = levelSettings.m_time;
  m_timeLeft = m_totalTime;
  m_bonusTime = 0;

  addItemsToScene();

  // center level item on scene
  centerLevelInView();

  return true;
}

void Level::addItemToLevel(QGraphicsItem *pItem)
{
  pItem->setParentItem(this);
}

void Level::addObjToLevel(GameObject *pObj)
{
  // do not re-add objects
  if(pObj->added())
    return;

  pObj->setParentItem(this);
  pObj->setAdded();

#ifdef SHOW_COL_RECTS
  pObj->m_colRectItem.setParentItem(this);
#endif

  if(curGameMode() == LevelEditMode)
  {
    m_limitsInitialized = true;

    if(gLevelElementList.size() + gSheepList.size() + gDogList.size() == 1)
    {
      // if this is the first object, this is simple:

      m_minPoint = pObj->getTopLeftPos();
      m_maxPoint = pObj->getBottomRightPos();
    }
    else
    {
      if(pObj->getTopLeftPos().x() < m_minPoint.x())
      {
        m_minPoint.setX(pObj->getTopLeftPos().x());
        qDebug("new min x: %.0f", m_minPoint.x());
      }
      if(pObj->getTopLeftPos().y() < m_minPoint.y())
      {
        m_minPoint.setY(pObj->getTopLeftPos().y());
        qDebug("new min y: %.0f", m_minPoint.y());
      }

      if(pObj->getBottomRightPos().x() > m_maxPoint.x())
      {
        m_maxPoint.setX(pObj->getBottomRightPos().x());
        qDebug("new max x: %.0f", m_maxPoint.x());
      }
      if(pObj->getBottomRightPos().y() > m_maxPoint.y())
      {
        m_maxPoint.setY(pObj->getBottomRightPos().y());
        qDebug("new max y: %.0f", m_maxPoint.y());
      }
    }
  }
}

void Level::addItemsToScene()
{
  // ------------------------------------------------------------------------
  // 0) add background decoration (grass, flowers, ...)
  // there's one decoration element randomly placed in each square of rndDecoSquareSize x rndDecoSquareSize pixels
  /*!!!
  unsigned long int rndDecoSquareSize = 80;
  for(unsigned long int ix = 0; ix <= qMax(gWindowWidth, m_width); ix += rndDecoSquareSize)
  {
    for(unsigned long int iy = 0; iy <= qMax(gWindowHeight, m_height); iy += rndDecoSquareSize)
    {
      Deco *pDeco = new Deco(ix + getRandom(10, rndDecoSquareSize), iy + getRandom(10, rndDecoSquareSize));
      pDeco->setParentItem(this);
    }
  }
  */

  qDebug("adding %d level elements...", gLevelElementList.size());

  // ------------------------------------------------------------------------
  // 1) add level elements (fences, bushes, ...) of layers 0 and 1 first
  for(unsigned int layer=0; layer<=1; layer++)
  {
    foreach(LevelElement *pElement, gLevelElementList)
    {
      if(pElement->m_layer != layer)
        continue;

      addObjToLevel(pElement);
    }
  }

  // ------------------------------------------------------------------------
  // 2) dog(s)
  // -> Adding before the sheep to make sure Dog::advance() is called before Sheep::advance().
  // -> Like this, collision bools (for col.s between dog and sheep) are already set when entering
  //    Sheep::advance()
  qDebug("adding %d dog(s)...", gDogList.size());
  foreach(Character *pDog, gDogList)
    addObjToLevel(pDog);

  // ------------------------------------------------------------------------
  // 3) add sheep to stage
  qDebug("adding %d sheep...", gSheepList.size());
  foreach(Character *pSheep, gSheepList)
  {
    ((Sheep*)pSheep)->init((void*)this);

    addObjToLevel(pSheep);
  }

  // ------------------------------------------------------------------------
  // 4.a) exit shack(s): foreground (layer 2)
  foreach(LevelElement *pElement, gLevelElementList)
  {
    if(pElement->m_layer != 2)
      continue;

    addObjToLevel(pElement);
  }

  // ------------------------------------------------------------------------
  // 4.b) trees etc.: foreground (layer 1)
  foreach(LevelElement *pElement, gLevelElementList)
  {
    if(pElement->m_layer != 3)
      continue;

    addObjToLevel(pElement);
  }

  // ------------------------------------------------------------------------
  // bonus stuff (invisible most of the time)
  p_bonus->setParentItem(this);

  qDebug("number of scene items after adding: %d", p_scene->items().size());
}

void Level::addGuiElements()
{
  if(m_guiElementsAdded)
  {
    qDebug("GUI elements already added to scene!");
  }
  else
  {
    // info elements
    p_scene->addItem(&m_infoItems);
    m_infoItems.setPos( (p_scene->width() - gWindowWidth) / 2,
                        (p_scene->height() - gWindowHeight) / 2 );

    // editor elements
    p_scene->addItem(&m_editorItems);
    m_editorItems.setPos( (p_scene->width() - gWindowWidth) / 2,
                          (p_scene->height() - gWindowHeight) / 2 );

    // level choose elements
    p_scene->addItem(&m_levelChooseItems);
    m_levelChooseItems.setPos( (p_scene->width() - gWindowWidth) / 2,
                               (p_scene->height() - gWindowHeight) / 2 );

    m_guiElementsAdded = true;
  }
}

void Level::removeGuiElements()
{
  if(m_guiElementsAdded)
  {
    qDebug("removing info items...");
    p_scene->removeItem(&m_infoItems);

    qDebug("removing editor items...");
    p_scene->removeItem(&m_editorItems);

    qDebug("removing level choose items...");
    p_scene->removeItem(&m_levelChooseItems);

    m_guiElementsAdded = false;
  }
  else
  {
    qDebug("GUI elements not added to scene -> nothing to remove");
  }
}

bool Level::start()
{
  m_infoItems.init();

  setVisible(true);
  m_infoItems.setVisible(true);
  m_editorItems.setVisible(false);
  m_levelChooseItems.setVisible(false);

  gDogScore = 0;

  m_numTotalSheep = gSheepList.size();
  m_numRemainingSheep = m_numTotalSheep;

  startOverviewZooming();

#ifndef QT_NO_SOUND
  if(prevGameMode() != LevelEditMode)
    levelSounds[m_levelNumber-1]->play();
#endif

  /*!!!
  if(music != NULL)
  {
    music->stop();
    delete music;
  }
  QString music_file;
  QTextStream(&music_file) << "sfx/music/music" << getRandom(1, kNumMusicTracks) << ".mp3";
  music = new QSound(music_file);
  */

  return true;
}

void Level::startOverviewZooming()
{
  if(p_scene->startZooming())
  {
    m_zoomChange = (gLevelZoomFactor - kOverviewStartZoom) / (kOverviewZoomDuration*kFrameRate);
    m_curZoom = kOverviewStartZoom;
  }
  else
  {
    qDebug("already overview zooming!");
  }
}

void Level::overviewZooming()
{
  if(m_curZoom >= gLevelZoomFactor)
  {
    p_scene->stopZooming();

    centerDogInView();

    p_scene->resumeGame();

    return;
  }

  setScale(m_curZoom);
  m_curZoom += m_zoomChange;

  // TODO: center whole level instead of dog
  centerLevelInView();
}

void Level::update()
{
  // 1) make sure the dog can be seen (scrolls if necessary)
  checkDogInView();

  // 2) update bark power
  m_infoItems.updateBarkPowerBar();

  // 3) update time / score / is there any time left?
  if(gCurFrameNumber % kFrameRate == 0)
    checkTime();

  // 4) handle bonus
  p_bonus->checkForShow();
}

void Level::minusOneSheep()
{
  m_numRemainingSheep--;

  gDogScore += kSheepScore;
  m_infoItems.updateScore();

  if(m_numRemainingSheep == 0)
    finish();
}

void Level::finish()
{
  if(m_numRemainingSheep > 0)
  {
    qDebug("level not finished yet (%ld sheep left)!", m_numRemainingSheep);
    return;
  }

  p_scene->pauseGame(false);
  p_scene->reset();

  p_bonus->resetMode();

  bool editMode = (prevGameMode() == LevelEditMode);
  QString text;
  if(editMode)
    text = QString("level finished!\n\nscore: %2\ntime bonus: %3\n\ntotal score: %4").arg(gDogScore).
                   arg(kTimeBonusFactor * m_timeLeft).arg(gDogScore + kTimeBonusFactor * m_timeLeft);
  else
    text = QString("level %1 finished!\n\nscore: %2\ntime bonus: %3\n\ntotal score: %4").arg(m_levelNumber).arg(gDogScore).
                   arg(kTimeBonusFactor * m_timeLeft).arg(gDogScore + kTimeBonusFactor * m_timeLeft);

  QList<QString> stringList;
  stringList.append(QString("play again"));
  if(editMode)
    stringList.append(QString("return to editor"));
  else
  {
    stringList.append(QString("next Level"));
    stringList.append(QString("stop playing"));
  }
  p_scene->showMessageBox(text, &stringList, finishClickedStep1, this);

  gDogScore += kTimeBonusFactor * m_timeLeft;
}

void Level::finishClickedStep1(ButtonContext *pButtonContext)
{
  ((Level*)pButtonContext->p_context)->finishClickedStep2(pButtonContext->buttonNumber);
}

void Level::finishClickedStep2(int button)
{
  if(button == 1)
  {
    resetCurrentLevel();
    goToGameMode(curGameMode());
  }
  else if(button == 2)
  {
    if(prevGameMode() == BuiltInChooserMode || prevGameMode() == UserChooserMode)
    {
      clear();
      m_levelNumber++;

      bool ok = false;
      if(prevGameMode() == BuiltInChooserMode)
        ok = loadLevel(BuiltInSource);
      else if(prevGameMode() == UserChooserMode)
        ok = loadLevel(UserSource);

      if(ok)
      {
        goToGameMode(PlayMode);
        return;
      }

      goToMainMenuMode();
    }
    else if(prevGameMode() == LevelEditMode)
    {
      restorePrevGameMode(false);
    }
    else
    {
      qDebug("invalid mode %d after level finished!", curGameMode());
      goToMainMenuMode();
    }
  }
  else if(button == 3)
  {
    restorePrevGameMode(false);
  }
  else
  {
    qDebug("unhandled button for unfinished level!");
  }
}

void Level::allLevelsDoneClickedStep1(ButtonContext *pButtonContext)
{
  ((Level*)pButtonContext->p_context)->finishClickedStep2(pButtonContext->buttonNumber);
}

void Level::allLevelsDoneClickedStep2(int button)
{
  restorePrevGameMode(false);
}

void Level::exitErrorClickedStep1(ButtonContext *pButtonContext)
{
  // TODO: send crash report...

  exit(EXIT_SUCCESS);
}

void Level::centerLevelInView()
{
  setPos((p_scene->width() - m_width*gLevelZoomFactor) / 2.0,
         (p_scene->height() - m_height*gLevelZoomFactor) / 2.0);

  //qDebug("centered level in view");
}

void Level::centerDogInView()
{
  if(!gDogList.empty())
  {
    Character *pDog = gDogList.at(0);
    QPointF dog_sceneCoord = pDog->scenePos() + pDog->relMidPoint();
    moveBy(kSceneWidth/2 - dog_sceneCoord.x(), kSceneHeight/2 - dog_sceneCoord.y());
  }
}

void Level::checkDogInView()
{
  if(gDogList.isEmpty())
  {
    qDebug("dog list empty!");
    return;
  }

  Character *pDog = gDogList.at(0);

  if(pDog->getSpeed() > kZeroSpeedThreshold)
  {
    // TODO: this calculation is not perfect:
    // changing the walking direction by 180� near the border causes a small jump in scrolling.
    // this is due to the change in the character's coordinate system when turning.

    QPointF dogViewCoord = pDog->absMidPoint()*gLevelZoomFactor + scenePos() - gSceneToViewOffset;
    QPointF levelTranslate(0, 0);

    if(dogViewCoord.x() < gDogMinDistToBorder + gInfoAreaSize)
      levelTranslate.setX(gDogMinDistToBorder + gInfoAreaSize - dogViewCoord.x());
    else if(dogViewCoord.x() > gWindowWidth - gDogMinDistToBorder)
      levelTranslate.setX(gWindowWidth - gDogMinDistToBorder - dogViewCoord.x());

    if(dogViewCoord.y() < gDogMinDistToBorder)
      levelTranslate.setY(gDogMinDistToBorder - dogViewCoord.y());
    else if(dogViewCoord.y() > gWindowHeight - gDogMinDistToBorder)
      levelTranslate.setY(gWindowHeight - gDogMinDistToBorder - dogViewCoord.y());

    if(!levelTranslate.isNull())
    {
#ifdef DEBUG
      qDebug("scrolling level by %3.1f, %3.1f", levelTranslate.x(), levelTranslate.y());
#endif
      moveBy(levelTranslate.x(), levelTranslate.y());
    }
  }
}

void Level::checkTime()
{
  if(m_bonusTime == 0)
    m_timeLeft--;
  else
    m_bonusTime--;

  m_infoItems.updateTime(m_timeLeft, m_totalTime, m_bonusTime);

  if(m_timeLeft == 0)
    timeIsUp();
}

void Level::timeIsUp()
{
  //!!!gTimeIsUpSound.play(0, 0);

#ifndef QT_NO_SOUND
  gTryAgainSound->play();
#endif

  QList<QString> stringList;
  stringList.append(QString("try again"));
  stringList.append(QString("return"));
  p_scene->showMessageBox(QString("time is up!"), &stringList, timeIsUpClickedStep1, this);
}

void Level::timeIsUpClickedStep1(ButtonContext *pButtonContext)
{
  ((Level*)pButtonContext->p_context)->timeIsUpClickedStep2(pButtonContext->buttonNumber);
}

void Level::timeIsUpClickedStep2(int button)
{
  if(button == 1)
  {
    resetCurrentLevel();
    //!!! ok?
    goToGameMode(curGameMode());
  }
  else
  {
    restorePrevGameMode(false);
  }
}

void Level::resetCurrentLevel()
{
  p_scene->pauseGame(false);
  p_scene->reset();

  p_bonus->resetMode();

  m_numRemainingSheep = m_numTotalSheep;
  m_bonusTime = 0;

  // restore characters
  foreach(Character *pDog, gDogList)
  {
    ((Dog*)pDog)->init();
  }
  foreach(Character *pSheep, gSheepList)
  {
    ((Sheep*)pSheep)->init();
  }
}

// *******************************************************************************************
//                      IMPLEMENTATIONS OF VIRTUAL GRAPHICSITEM METHODS
// *******************************************************************************************

QRectF Level::boundingRect() const
{
  return QRectF(x(), y(), m_width, m_height);
}

void Level::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{

}
