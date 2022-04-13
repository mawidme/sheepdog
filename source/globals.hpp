#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include "level_types.hpp"
#include "types.hpp"

#include <QTimer>
#include <QtGlobal>
#include <QPixmap>
#include <QPen>
#include <QBrush>
#include <QSound>
#include <QFont>
#include <QScrollBar>
#include <QtSvg/QGraphicsSvgItem>

#ifdef __ANDROID__
#define QT_NO_SOUND
#define MOBILE_VERSION
#define ANDROID_MOBILE_VERSION
static const qreal kDefaultLevelZoomFactor = 1.0;
static const long int kDefaultWindowWidth = 800;
static const long int kDefaultWindowHeight = 480;

#elif defined(__symbian__) || defined(__SYMBIAN32__)
#define QT_SOUND
#define SYMBIAN_MOBILE_VERSION
#define MOBILE_VERSION
static const qreal kDefaultLevelZoomFactor = 1.0;
static const long int kDefaultWindowWidth = 640;
static const long int kDefaultWindowHeight = 360;

#else
#define QT_SOUND
#undef MOBILE_VERSION
static const qreal kDefaultLevelZoomFactor = 1.0;
static const long int kDefaultWindowWidth = 800;
static const long int kDefaultWindowHeight = 480;
#endif

// the symbian compiler doesn't seem to like this define because of the ellipsis
#ifndef SYMBIAN_MOBILE_VERSION
// Activate for debugging output etc.
#ifndef DEBUG_OUTPUT
#define qDebug(...) \
				do {}       \
				while(0)
#endif
#endif //!SYMBIAN_MOBILE_VERSION

// Define to show collision rectangles
//#define SHOW_COL_RECTS

// Sheep collide with each other if defined
//#define SHEEP_TO_SHEEP_COLLISION


extern QApplication *gApplication;


// ===========================================================================
// GUI / MAIN

extern void *gLevel;

// changed when displaying main menu, editing level, normal game, ...
extern QList<GameMode> gModeList;
extern GameMode gNextMode;

extern bool gShowingMsgBox;

extern EditType gEditMode;

static const int kMaxNumBoxButtons = 3;

extern qreal gScreenShrinkFactor;

extern bool gUseMouseEvents;

static const qreal kMinViewZoomFactor = 0.3;
static const qreal kMaxViewZoomFactor = 3.0;

extern qreal gLevelZoomFactor;

static const long unsigned int kInvalidUint = 0xffffffff;

// set to true while pinch zooming
extern bool gPinchZooming;

// dimensions of whole game area
static const long int kSceneWidth = 10240;
static const long int kSceneHeight = 10240;

static const qreal kDefaultGuiSpacer = 5.0;
extern qreal gGuiSpacer;

static const long int kMinScreenWidth = 320;
static const long int kMinScreenHeight = 240;

static const qreal kInfoBarToFontFactor = 1.5;

extern qreal gInfoBarLength;
extern qreal gInfoBarWidth;

extern qreal gBarToLabelOffsetX;
extern qreal gBarToLabelOffsetY;

extern QFont gInfoFont;
extern qreal gInfoFontSize;
extern qreal gEffectiveInfoFontSize;

static const qreal kEffectiveFontSizeFactor = 25.0 / 18.0;

static const int kInfoAlpha = 100;
static const QColor kInfoColor(0, 0, 0, kInfoAlpha);

extern QColor gBackgroundColor;

extern int gScreenWidth;
extern int gScreenHeight;

// The window dimensions equal the screen dimensions for phones as there fullscreen mode is set.
extern qreal gWindowWidth;
extern qreal gWindowHeight;

extern QPointF gViewToSceneMap;

static const qreal kDefaultDistBarkPowerLabelToBar = 85.0;
extern qreal gDistBarkPowerLabelToBar;

static const qreal kDefaultDistTimeLabelToBar = 50.0;
extern qreal gDistTimeLabelToBar;

extern QPointF gSceneToViewOffset;

extern QPen   gInfoBackgroundPen;
extern QBrush gInfoBackgroundBrush;

extern QPen   gInactiveBackgroundPen;
extern QBrush gInactiveBackgroundBrush;

extern QPen   gBarkBarPen;
extern QBrush gBarkBarBrush;

extern QPen   gTimeBarPen;
extern QBrush gTimeBarBrush;

extern QPen   gBonusTimeBarPen;
extern QBrush gBonusTimeBarBrush;

extern GraphicQuality gGraphicQuality;

extern long unsigned int gCurFrameNumber;

static const int kDefaultKeyCodeUp1    = Qt::Key_Up;
static const int kDefaultKeyCodeRight1 = Qt::Key_Right;
static const int kDefaultKeyCodeDown1  = Qt::Key_Down;
static const int kDefaultKeyCodeLeft1  = Qt::Key_Left;
static const int kDefaultKeyCodeBark1  = Qt::Key_Space;

static const int kDefaultKeyCodeUp2    = Qt::Key_W;
static const int kDefaultKeyCodeRight2 = Qt::Key_D;
static const int kDefaultKeyCodeDown2  = Qt::Key_S;
static const int kDefaultKeyCodeLeft2  = Qt::Key_A;
static const int kDefaultKeyCodeBark2  = Qt::Key_Space;

extern int gKeyCodeUp1;
extern int gKeyCodeRight1;
extern int gKeyCodeDown1;
extern int gKeyCodeLeft1;
extern int gKeyCodeBark1;

extern int gKeyCodeUp2;
extern int gKeyCodeRight2;
extern int gKeyCodeDown2;
extern int gKeyCodeLeft2;
extern int gKeyCodeBark2;

static const long int kFrameRate = 24;

// ---------------------------------------------------------------------------
// SCORE
static const int kSheepScore = 10;
static const qreal kTimeBonusFactor = 1.0;

extern long unsigned int gDogScore;


// ===========================================================================
// CHARACTERS

// spatial interval with which to check for character collisions
extern qreal gColCheckStepDiff;
// Highest spatial resolution to be handled (e.g. used when finding exact walking distance until collision)
static const qreal kSpeedResolution = 1.0;
// number of main walking directions (orthogonal system)
static const unsigned int kNumWalkDirections = 4;

#ifdef SHOW_COL_RECTS
extern QPen   kInactiveColRectPen;
extern QBrush kInactiveColRectBrush;
extern QPen   kActiveColRectPen;
extern QBrush kActiveColRectBrush;
#endif

extern long unsigned int gCharIdCounter;
static const qreal kColRectWidth = 5.0;


// ===========================================================================
// DOG

// TODO: to be optimized!
static const qreal kDogBarkClickThreshold = 30.0;

extern qreal gDogMinDistToBorder;

static const qreal kZeroSpeedThreshold = 0.01;
static const qreal kNormalDogSpeed = 11.0;
static const qreal kDogTargetReachedDistThreshold = kNormalDogSpeed;
static const QPointF kDogInitTargetPoint(10000.0, 10000.0);

static const qreal kMaxBarkPower = 100;
static const qreal kNeededBarkPower = 20;

static const qreal kDefaultMaxBarkDist = 200.0;

extern qreal gBarkPowerReload;
extern qreal gMaxBarkDist;
extern qreal gBarkIntensityConstant;
extern qreal gCurDogSpeed;


// ===========================================================================
// SHEEP

// bark influence radius
static const long int kSheepOnSpeedFactor = 2;
// blocked sheep looks for way out every second
static const long int kBlockedSheepNumFrameSkip = kFrameRate;

static const qreal kNormalSheepSpeed = 1.5;

extern qreal gMaxSheepSpeed;
extern qreal gNormalSheepSpeed;
extern qreal gSheepSlowDownValue;


// ===========================================================================
// LEVEL

// time bonus items stay
static const float kSurpriseShowTime = 3.5;
// minimum delay until next bonus appears
static const long unsigned int kBonusMinDelay = 6;
// maximum delay until next bonus appears
static const long unsigned int kBonusMaxDelay = 11;

const long unsigned int kTimeBonus = 15;

// if distance to object is greater than current obect's width multiplied
// with widthSkipFactor, skip collision check for that object
static const long int kWidthSkipFactor = 2;

const qreal kMinWaterSize = 20.0;

// do not take files from QT resources for eclipse debug build
#if defined(DEBUG_OUTPUT) && !defined(MOBILE_VERSION)
#define FILE_PREFIX ""
#define GET_FILE(x) x
#else
#define FILE_PREFIX ":/"
#define GET_FILE(x) ":/" x
#endif

#ifdef ANDROID_MOBILE_VERSION
#define GET_USER_LEVEL_FILE(x) "/sdcard/sheepdog/levels/"x
#define GET_SD_DIR(x)         "/sdcard/"x

#elif defined(SYMBIAN_MOBILE_VERSION)
#define GET_USER_LEVEL_FILE(x) "E:/sheepdog/levels/"x
#define GET_SD_DIR(x)         "E:/"x

#else // desktop version
#define GET_USER_LEVEL_FILE(x) "levels/user/" x
#endif


// ===========================================================================
// GRAPHICS

// ----------------------------------------------------------------------------------
// GUI

static const qreal kDefaultInfoAreaSizeMouse = 72;
static const qreal kDefaultInfoAreaSizeNoMouse = 40;

extern qreal gInfoAreaSize;

extern QGraphicsSvgItem *gBarkButtonItem;
extern QGraphicsSvgItem *gPauseButtonItem;

extern QGraphicsSvgItem *gRetryButtonItem;
extern QGraphicsSvgItem *gPlayButtonItem;
extern QGraphicsSvgItem *gStopButtonItem;

extern QRectF gBarkButtonRect;
extern QRectF gPauseButtonRect;

extern QRectF gRetryButtonRect;
extern QRectF gPlayButtonRect;
extern QRectF gStopButtonRect;

extern QGraphicsRectItem *gInactiveBackground;

// bark power and time bar are displayed vertically if set (default = true)
extern bool gVerticalBars;

// ----------------------------------------------------------------------------------
// CHARACTERS
extern QPixmap **gPngCharacterImg[NumCharacterTypes];
extern QSvgRenderer **gSvgCharacterRenderer[NumCharacterTypes];

static const int kNumAniStepsDog = 7;
static const int kNumAniStepsSheep = 1;

static const qreal kBarkWaveWidth = 25.0;
extern QBrush gBarkWaveBrush;
extern QPen gBarkWavePen;

// ----------------------------------------------------------------------------------
// LEVEL ELEMENTS
static const int kNumDecoStyles = 1;
static const int kNumExitStyles = 1;
static const int kNumFenceStyles = 1;
static const int kNumWaterStyles = 1;
static const int kNumTreeStyles = 1;
static const int kNumBushStyles = 1;

extern int gNumLevelElmtStyles[NumLevelElemTypes];

extern QPixmap **gPngLevelImg[NumLevelElemTypes];
extern QSvgRenderer **gSvgLevelRenderer[NumLevelElemTypes];

static const qreal kFenceWidth = 13;
static const qreal kFenceSlatWidth = 7;

extern QPen   gFenceSlatPen;
extern QBrush gFenceSlatBrush;
extern QPen   gFencePolePen;
extern QBrush gFencePoleBrush;

// ===========================================================================
// SOUNDS

#ifndef QT_NO_SOUND
// level
static const int kNumSheepRescuedSounds = 2;
extern QSound **gSheepRescuedSound;

static const int kNumLevelSounds = 4;
extern QSound **levelSounds;

static const int kNumMusicTracks = 4;

extern QSound *gLevelDoneSound;
extern QSound *gTryAgainSound;
extern QSound *gGameOverSound;

// sheep
static const int kNumSheepSounds = 4;
extern QSound **gSheepSound;

// dog
extern QSound *gBarkPowerCriticalSound;
extern QSound *gBarkPowerFullSound;
static const int kNumBarkSounds = 4;
extern QSound **gBarkSounds;
extern QSound *gNoBarkSound;
#endif // !QT_NO_SOUND

/*!!!
const int kAudioSampleRate = 22050;
const int kAudioSampleSize = 16;
const int kAudioChannelCount = 1;
const QString kAudioCodec("audio/pcm");
*/


#endif // GLOBALS_HPP
