#include "globals.hpp"

#include "level.hpp"

#include <Qt>


// TODO: free globals!

QApplication *gApplication = NULL;

// ===========================================================================
// GUI / MAIN

void *gLevel = NULL;

// ===========================================================================
// Factor by which everything should be shrinked, i.e. factor x < 1 => screen height is factor x smaller than
// a) 480px for touchscreen phones
// b) 240px for non-touchscreen phones
qreal gScreenShrinkFactor = 1.0;

bool gUseMouseEvents = true;
qreal gInfoAreaSize = kDefaultInfoAreaSizeMouse;

QList<GameMode> gModeList;
GameMode gNextMode = MainMenuMode;

bool gShowingMsgBox = false;

EditType gEditMode = FenceEdit;

// Deactivate controls while pinch zooming, as multitouch gestures also trigger regular mouse press events
// which aren't needed here.
bool gPinchZooming = false;

bool gVerticalBars = true;

qreal gDistBarkPowerLabelToBar = kDefaultDistBarkPowerLabelToBar;
qreal gDistTimeLabelToBar = kDefaultDistTimeLabelToBar;

qreal gLevelZoomFactor = kDefaultLevelZoomFactor;

qreal gWindowWidth = kDefaultWindowWidth;
qreal gWindowHeight = kDefaultWindowHeight;

int gScreenWidth = gWindowWidth;
int gScreenHeight = gWindowHeight;

QPointF gViewToSceneMap(0, 0);

qreal gGuiSpacer;

qreal gInfoBarLength;
qreal gInfoBarWidth;

qreal gBarToLabelOffsetX;
qreal gBarToLabelOffsetY;

QFont gInfoFont;
// size to assign to info text
qreal gInfoFontSize;
// effectively visible info text size in pixels
qreal gEffectiveInfoFontSize;

QColor gBackgroundColor(0, 100, 0);

// ===========================================================================
QPointF gSceneToViewOffset;

QPen   gInfoBackgroundPen(QColor(255, 255, 255, 0));
QBrush gInfoBackgroundBrush(QColor(255, 255, 255, kInfoAlpha));

QPen   gInactiveBackgroundPen(QColor(0, 0, 0, 150));
QBrush gInactiveBackgroundBrush(QColor(0, 0, 0, 150));
QGraphicsRectItem *gInactiveBackground = NULL;

QPen   gBarkBarPen(QColor(0, 0, 150, kInfoAlpha));
QBrush gBarkBarBrush(QColor(0, 0, 180, kInfoAlpha));

QPen   gTimeBarPen(QColor(0, 80, 0, kInfoAlpha));
QBrush gTimeBarBrush; // set in MainGame::initColors() -> gradient!

QPen   gBonusTimeBarPen(QColor(9, 150, 9, kInfoAlpha));
QBrush gBonusTimeBarBrush(QColor(9, 150, 9, kInfoAlpha));

//!!! load from settings file!
GraphicQuality gGraphicQuality = MedQuality;

long unsigned int gCurFrameNumber = 0;

int gKeyCodeUp1    = kDefaultKeyCodeUp1;
int gKeyCodeRight1 = kDefaultKeyCodeRight1;
int gKeyCodeDown1  = kDefaultKeyCodeDown1;
int gKeyCodeLeft1  = kDefaultKeyCodeLeft1;
int gKeyCodeBark1  = kDefaultKeyCodeBark1;

int gKeyCodeUp2    = kDefaultKeyCodeUp2;
int gKeyCodeRight2 = kDefaultKeyCodeRight2;
int gKeyCodeDown2  = kDefaultKeyCodeDown2;
int gKeyCodeLeft2  = kDefaultKeyCodeLeft2;
int gKeyCodeBark2  = kDefaultKeyCodeBark2;

// ===========================================================================
// CHARACTERS

qreal gColCheckStepDiff = 10000.0;

#ifdef SHOW_COL_RECTS
QPen   kInactiveColRectPen   = QPen(QColor(0, 150, 0));
QBrush kInactiveColRectBrush = QBrush(QColor(0xff, 0xb4, 0x39, 100));
QPen   kActiveColRectPen     = QPen(QColor(150, 0, 0));
QBrush kActiveColRectBrush   = QBrush(QColor(200, 0, 0));
#endif

long unsigned int gCharIdCounter = 0;


// ===========================================================================
// DOG

qreal gDogMinDistToBorder;
qreal gCurDogSpeed = kNormalDogSpeed;
qreal gBarkPowerReload = 0.17;
qreal gMaxBarkDist = kDefaultMaxBarkDist;
qreal gBarkIntensityConstant = 250;

long unsigned int gDogScore = 0;


// ===========================================================================
// SHEEP

qreal             gNormalSheepSpeed = kNormalSheepSpeed;
qreal             gMaxSheepSpeed = kNormalDogSpeed;
qreal             gSheepSlowDownValue = 0.06;


// ===========================================================================
// GRAPHICS

// CONTROLS

QGraphicsSvgItem *gBarkButtonItem = NULL;
QGraphicsSvgItem *gPauseButtonItem = NULL;

QRectF gBarkButtonRect;
QRectF gPauseButtonRect;

// normally invisible - will only be displayed when game is paused.
QGraphicsSvgItem *gRetryButtonItem = NULL;
QGraphicsSvgItem *gPlayButtonItem = NULL;
QGraphicsSvgItem *gStopButtonItem = NULL;

QRectF gRetryButtonRect;
QRectF gPlayButtonRect;
QRectF gStopButtonRect;

// CHARACTERS
QPixmap **gPngCharacterImg[NumCharacterTypes] = {NULL};
QSvgRenderer **gSvgCharacterRenderer[NumCharacterTypes] = {NULL};

QBrush gBarkWaveBrush(QColor(0, 0, 0, 0));
QPen   gBarkWavePen(QColor(0, 0, 0, 20));

// LEVEL ELEMENTS
QPen   gFenceSlatPen(QColor(96, 48, 16));
QBrush gFenceSlatBrush(QColor(135, 73, 33));
QPen   gFencePolePen(QColor(76, 42, 20));
QBrush gFencePoleBrush(QColor(110, 53, 18));

int gNumLevelElmtStyles[NumLevelElemTypes];

QPixmap **gPngLevelImg[NumLevelElemTypes] = {NULL};
QSvgRenderer **gSvgLevelRenderer[NumLevelElemTypes] = {NULL};


// ===========================================================================
// SOUNDS

#ifndef QT_NO_SOUND
// level
QSound **levelSounds            = NULL;
QSound *gLevelDoneSound         = NULL;
QSound *gTryAgainSound          = NULL;
QSound *gGameOverSound          = NULL;
QSound **gSheepRescuedSound     = NULL;

// sheep
QSound **gSheepSound            = NULL;

// dog
QSound *gBarkPowerCriticalSound = NULL;
QSound *gBarkPowerFullSound     = NULL;
QSound **gBarkSounds            = NULL;
QSound *gNoBarkSound            = NULL;
#endif // QT_SOUND
