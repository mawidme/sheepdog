#ifndef BONUS_HPP
#define BONUS_HPP

#include "mygraphicsscene.hpp"
#include "mygraphicsview.hpp"

#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QtSvg/QGraphicsSvgItem>
#include <QGraphicsBlurEffect>

//TODO: SuperDogMode (Dog: setOpacity(0.5), setZValue(<top>))
enum BonusMode
{
  DrunkMode = 0,
  SleepMode,
  SheepOnSpeedMode,
  TimeBonusMode,
  PowerBarkMode,
  NumBonusModes,
  NoBonusMode
};

class Bonus : public QGraphicsItem
{
public:
  Bonus();
  ~Bonus();

  bool init(void *pLevel, MyGraphicsView *pView, MyGraphicsScene *pScene);
  void initImg();

  void pickMode();
  bool modeIsValid() const;

  void checkForShow();

  void resetMode();

  // implementations of virtual QGraphicsItem methods
  QRectF boundingRect() const;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
  void delImg();

  void doBonusStuff();

  void showBox();
  void hideBox();

  void showMode();
  void hideMode();

  void setNextBonusFrames();

  void            *p_level;
  MyGraphicsView  *p_view;
  MyGraphicsScene *p_scene;

  bool m_imgInit;

  QRectF m_colRect;

  BonusMode m_curMode;

  long unsigned int  m_boxShowFrame;
  long unsigned int  m_surpriseHideFrame;

  long unsigned int  m_curModeImageHideFrame;

  long unsigned int  m_curModeActivateFrame;
  long unsigned int  m_curModeDeactivateFrame;

  bool m_surprise;
  bool m_boxVisible;
  bool m_bonusPickedUp;
  bool m_modeVisible;

  QPointF m_curModeImageMidPoint;

  // While the current mode's image is being displayed, this counts from 0.0 to 1.0.
  qreal  m_curModeImageOffset;
  // This is added to the offset each frame.
  qreal  m_curModeImageOffsetDelta;

  ImageType m_imgType;
  QGraphicsItem       *p_boxImgItem;
  QPixmap             *p_boxPngImg;
  QGraphicsPixmapItem *p_boxPngImgItem;
  QGraphicsSvgItem    *p_boxSvgImgItem;

  QGraphicsItem       *p_surpriseImgItem;
  QPixmap             *p_surprisePngImg;
  QGraphicsPixmapItem *p_surprisePngImgItem;
  QGraphicsSvgItem    *p_surpriseSvgImgItem;

  QGraphicsItem       *p_modeImgItem[NumBonusModes];
  QPixmap             *p_modePngImg[NumBonusModes];
  QGraphicsPixmapItem *p_modePngImgItem[NumBonusModes];
  QGraphicsSvgItem    *p_modeSvgImgItem[NumBonusModes];

  QGraphicsItem       *p_boxModeImgItem[NumBonusModes];
  QPixmap             *p_boxModePngImg[NumBonusModes];
  QGraphicsPixmapItem *p_boxModePngImgItem[NumBonusModes];
  QGraphicsSvgItem    *p_boxModeSvgImgItem[NumBonusModes];

  // BONUS SPECIFIC MEMBERS

  qreal m_oldBarkReload;
  qreal m_oldBarkMaxDist;
  qreal m_oldBarkIntConst;
  qreal m_oldSheepMaxSpeed;
};

#endif // BONUS_HPP
