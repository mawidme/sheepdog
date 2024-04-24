#ifndef LEVEL_HPP
#define LEVEL_HPP

#include "game_object.hpp"

#include "exit.hpp"

#include "level_element.hpp"
#include "mygraphicsscene.hpp"
#include "bonus.hpp"

#include "dog.hpp"
#include "sheep.hpp"

#include "info_items.hpp"
#include "editor_items.hpp"
#include "level_choose_items.hpp"

#include "globals.hpp"

#include "mygraphicsview.hpp"

#if defined(QT_SOUND)
#include <QtMultimedia/QSoundEffect>
#endif

#include <QtCore/QTimer>

class Level : public QGraphicsItem
{
public:
	Level();
	~Level();

  void clear();
  void removeGameObjects();

  void menu();
  static void mainMenuClickedStep1(ButtonContext *pButtonContext);
  void mainMenuClickedStep2(int button);

  bool showChooserDialog(const LevelSource source);
  void chooserClick(QPointF scenePoint);
  bool loadLevel(const LevelSource source);

  void initEditor();
  void editor();
  void editClick(QPointF scenePoint, QPointF levelPoint);
  void editClickButton(int type);
  void editClickLevel(QPointF levelPoint);

  void editMove(QPointF levelPoint, QPointF delta);
  void editRelease();

  void leaveEditor();
  void askForSave();
  static void askForSaveClickedStep1(ButtonContext *pButtonContext);
  void askForSaveClickedStep2(int button);

	bool init();

	void setupSceneAndView();

  bool start();

  void startOverviewZooming();
  void overviewZooming();

  void addItemToLevel(QGraphicsItem *pItem);
  void addObjToLevel(GameObject *pObj);

  void update();

  void minusOneSheep();

  void finish();
  static void finishClickedStep1(ButtonContext *pButtonContext);
  void finishClickedStep2(int button);

  static void allLevelsDoneClickedStep1(ButtonContext *pButtonContext);
  void allLevelsDoneClickedStep2(int button);

  static void exitErrorClickedStep1(ButtonContext *pButtonContext);

  void centerLevelInView();

  void centerDogInView();
  void checkDogInView();

  void checkTime();
  void timeIsUp();
  static void timeIsUpClickedStep1(ButtonContext *pButtonContext);
  void timeIsUpClickedStep2(int button);

  void resetCurrentLevel();

  // These are just dummies to allow items to be added to Level as main graphics item.
  QRectF boundingRect() const;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

  // dimensions in pixels
	qreal m_width;
	qreal m_height;
	
	long unsigned int m_timeLeft;   // seconds
	long unsigned int m_totalTime;  // seconds
  long unsigned int m_bonusTime;  // seconds

	long unsigned int m_levelNumber;

	long int m_numTotalSheep;
	long int m_numRemainingSheep;

  // ----------------------------------------------------------------------------------------------------
  // INFOS / CONTROLS
  InfoItems           m_infoItems;
  EditorItems         m_editorItems;
  LevelChooseItems    m_levelChooseItems;

private:
  // ----------------------------------------------------------------------------------------------------
  // MAIN WINDOW / SCENE

  void addItemsToScene();
  void addGuiElements();
  void removeGuiElements();

  int checkLevel();
  int saveLevel();

  void moveLevelBy(QPointF delta);
  void editSearchLimits();
  bool adjustLimits();
  void editCheckEmpty();
  void editPrepareLevel();
  bool editAddFence();
  bool editAddWater(QPointF levelPoint);
  bool editAddImgObj();
  void editErase();
  bool removeItemFromUndoList(GameObjType type, int num);

  MyGraphicsScene *p_scene;
  MyGraphicsView  *p_view;

  bool m_guiElementsAdded;

  bool m_viewInitDone;

  qreal m_zoomChange;
  qreal m_curZoom;

  Bonus *p_bonus;


  // EDIT MODE STUFF (all in level coordinates):

  bool m_dragEdit;
  // set to true when level limits set (and false otherwise)
  bool m_limitsInitialized;
  // point of mouse click
  QPointF m_editClickPoint;
  // list of added object types for undoing
  QList<GameObjType> m_undoList;
  // "minimum and maximum coordinates" (top left / bottom right)
  QPointF m_minPoint;
  QPointF m_maxPoint;
  // is set to true if there are unsaved changes in edited level
  bool m_askForSave;
  // is set when a new level is created
  bool m_rescanLevels;
  // "drawn" level item (e.g. fence, water) is currently being edited and on screen
  bool m_drawingItem;
};

#endif // LEVEL_HPP
