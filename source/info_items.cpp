#include "info_items.hpp"
#include "globals.hpp"
#include "global_lists.hpp"

#include "dog.hpp"

#include <QtGui/QTextDocument>


InfoItems::InfoItems() : m_initialized(false)
{
}

InfoItems::~InfoItems()
{
  if(gInactiveBackground != NULL)
    delete gInactiveBackground;
}

QRectF InfoItems::boundingRect() const
{
  return QRectF(pos().x(), pos().y(), gWindowWidth, gWindowHeight);
}

void InfoItems::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{

}

void InfoItems::init()
{
  if(!m_initialized)
  {
    setTextAndColors();
    setGeometry();

    m_initialized = true;
  }
}

void InfoItems::setTextAndColors()
{
  // a) colors / graphics
  m_infosBackground.setPen(gInfoBackgroundPen);
  m_infosBackground.setBrush(gInfoBackgroundBrush);

  if(gInactiveBackground == NULL)
  {
    gInactiveBackground = new QGraphicsRectItem();
    gInactiveBackground->setPen(gInactiveBackgroundPen);
    gInactiveBackground->setBrush(gInactiveBackgroundBrush);
    gInactiveBackground->setParentItem(this);
  }

  m_barkPowerBar.setPen(gBarkBarPen);
  m_barkPowerBar.setBrush(gBarkBarBrush);

  m_timeBar.setPen(gTimeBarPen);
  m_timeBar.setBrush(gTimeBarBrush);

  m_bonusTimeBar.setPen(gBonusTimeBarPen);
  m_bonusTimeBar.setBrush(gBonusTimeBarBrush);

  // b) text
  m_barkPowerLabel.setDefaultTextColor(kInfoColor);
  if(gVerticalBars)
    m_barkPowerLabel.setPlainText(QString("B"));
  else
    m_barkPowerLabel.setPlainText(QString("BARK POWER:"));

  m_timeLabel.setDefaultTextColor(kInfoColor);
  if(gVerticalBars)
    m_timeLabel.setPlainText(QString("T"));
  else
    m_timeLabel.setPlainText(QString("TIME:"));

  m_scoreLabel.setDefaultTextColor(kInfoColor);
  m_scoreLabel.setPlainText(QString("SCORE:"));

  m_scoreCounter.setDefaultTextColor(kInfoColor);
  m_scoreCounter.setPlainText(QString("%1").arg(gDogScore));

  // e) parent
  m_infosBackground.setParentItem(this);
  gInactiveBackground->setParentItem(this);

  m_barkPowerLabel.setParentItem(this);
  m_timeLabel.setParentItem(this);
  m_scoreLabel.setParentItem(this);
  m_scoreCounter.setParentItem(this);
  m_barkPowerBar.setParentItem(this);
  m_timeBar.setParentItem(this);
  m_bonusTimeBar.setParentItem(this);

  gBarkButtonItem->setParentItem(this);
  gPauseButtonItem->setParentItem(this);

  gRetryButtonItem->setParentItem(this);
  gPlayButtonItem->setParentItem(this);
  gStopButtonItem->setParentItem(this);

  gInactiveBackground->setVisible(false);

  gRetryButtonItem->setVisible(false);
  gPlayButtonItem->setVisible(false);
  gStopButtonItem->setVisible(false);
}

void InfoItems::setGeometry()
{
  // Vertical offset for info element background
  qreal yOffset = gWindowHeight - gInfoAreaSize;

  // 1.a) button images
  QSizeF inGameButtonSize = gScreenShrinkFactor * gBarkButtonItem->boundingRect().size();
  gBarkButtonItem->setScale(gScreenShrinkFactor);
  gPauseButtonItem->setScale(gScreenShrinkFactor);
  if(gVerticalBars)
  {
    gPauseButtonItem->setPos(gGuiSpacer, gGuiSpacer);
    gBarkButtonItem->setPos(gPauseButtonItem->x(), gWindowHeight - inGameButtonSize.height() - gGuiSpacer);
  }
  else
  {
    gBarkButtonItem->setPos(3.0 * gGuiSpacer,
                            yOffset + (gInfoAreaSize - inGameButtonSize.height()) / 2.0);
    gPauseButtonItem->setPos(gWindowWidth - inGameButtonSize.width() - (3.0 * gGuiSpacer),
                             gBarkButtonItem->y());
  }

  QSizeF gamePausedButtonSize = gScreenShrinkFactor * gRetryButtonItem->boundingRect().size();
  gStopButtonItem->setScale(gScreenShrinkFactor);
  gStopButtonItem->setPos(gWindowWidth / 2.0 - 2.0 * gamePausedButtonSize.width(),
                          (gWindowHeight - gInfoAreaSize - gamePausedButtonSize.height()) / 2.0);
  gRetryButtonItem->setScale(gScreenShrinkFactor);
  gRetryButtonItem->setPos(gWindowWidth / 2.0 - 0.5 * gamePausedButtonSize.width(),
                           gStopButtonItem->y());
  gPlayButtonItem->setScale(gScreenShrinkFactor);
  gPlayButtonItem->setPos(gWindowWidth / 2.0 + gamePausedButtonSize.width(),
                          gStopButtonItem->y());

  // 1.b) button rectangles
  gBarkButtonRect = QRectF(gBarkButtonItem->boundingRect());
  gBarkButtonRect.setSize(inGameButtonSize);
  gBarkButtonRect.translate(gBarkButtonItem->pos() + gSceneToViewOffset);

  gPauseButtonRect = QRectF(gPauseButtonItem->boundingRect());
  gPauseButtonRect.setSize(inGameButtonSize);
  gPauseButtonRect.translate(gPauseButtonItem->pos() + gSceneToViewOffset);

  gRetryButtonRect = QRectF(gRetryButtonItem->boundingRect());
  gRetryButtonRect.setSize(gamePausedButtonSize);
  gRetryButtonRect.translate(gRetryButtonItem->pos() + gSceneToViewOffset);

  gPlayButtonRect = QRectF(gPlayButtonItem->boundingRect());
  gPlayButtonRect.setSize(gamePausedButtonSize);
  gPlayButtonRect.translate(gPlayButtonItem->pos() + gSceneToViewOffset);

  gStopButtonRect = QRectF(gStopButtonItem->boundingRect());
  gStopButtonRect.setSize(gamePausedButtonSize);
  gStopButtonRect.translate(gStopButtonItem->pos() + gSceneToViewOffset);

  // 2) fonts
  m_barkPowerLabel.setFont(gInfoFont);
  m_timeLabel.setFont(gInfoFont);
  m_scoreLabel.setFont(gInfoFont);
  m_scoreCounter.setFont(gInfoFont);

  // 3) spacers and rectangles
  if(gVerticalBars)
    m_infosBackground.setRect(0.0, 0.0, gInfoAreaSize, gWindowHeight + 4.0);
  else
    m_infosBackground.setRect(0.0, 0.0, gWindowWidth + 4.0, gInfoAreaSize);

  gInactiveBackground->setRect(0.0, 0.0, gWindowWidth, gWindowHeight);

  if(gVerticalBars)
    m_barkPowerBar.setRect(0.0, 0.0, gInfoBarWidth, gInfoBarLength);
  else
    m_barkPowerBar.setRect(0.0, 0.0, gInfoBarLength, gInfoBarWidth);

  gDistTimeLabelToBar = m_timeLabel.document()->size().width() + gGuiSpacer;

  if(gVerticalBars)
    m_timeBar.setRect(0.0, 0.0, gInfoBarWidth, gInfoBarLength);
  else
    m_timeBar.setRect(0.0, 0.0, gInfoBarLength, gInfoBarWidth);

  m_bonusTimeBar.setVisible(false);

  // 4) positions
  if(gUseMouseEvents)
  {
    if(gVerticalBars)
    {
      m_infosBackground.setPos(-2.0, -2.0);

      //!!! fix calculation
      gDistBarkPowerLabelToBar = m_barkPowerLabel.document()->size().width() + gGuiSpacer;

      m_scoreLabel.setPos(gGuiSpacer,
                          gGuiSpacer + gPauseButtonRect.height());
      m_scoreCounter.setPos(m_scoreLabel.x() + gDistBarkPowerLabelToBar,
                            m_scoreLabel.y() + 3.0 * gGuiSpacer);

      m_barkPowerLabel.setPos(3.0 * gGuiSpacer,
                              m_scoreCounter.y() + m_scoreCounter.document()->size().height() + gGuiSpacer);
      m_barkPowerBar.setPos(m_barkPowerLabel.x() + gBarToLabelOffsetX,
                            m_barkPowerLabel.y() + 4.0 * gGuiSpacer);

      m_timeLabel.setPos(m_barkPowerLabel.x() + 4.0 * gGuiSpacer,
                         m_barkPowerLabel.y());
      m_timeBar.setPos(m_timeLabel.x() + gBarToLabelOffsetX,
                       m_timeLabel.y() + 4.0 * gGuiSpacer);
    }
    else
    {
      m_infosBackground.setPos(-2.0, yOffset);

      gDistBarkPowerLabelToBar = m_barkPowerLabel.document()->size().width() + gGuiSpacer;

      m_barkPowerLabel.setPos(7.0 * gGuiSpacer + inGameButtonSize.width(),
                              (gGuiSpacer / 2.0) + yOffset);
      m_barkPowerBar.setPos(m_barkPowerLabel.x() + gDistBarkPowerLabelToBar,
                            m_barkPowerLabel.y() + gBarToLabelOffsetY);

      m_timeLabel.setPos(m_barkPowerLabel.x(),
                         m_barkPowerLabel.y() + gEffectiveInfoFontSize + (gGuiSpacer / 2.0));
      m_timeBar.setPos(m_barkPowerBar.x(),
                       m_timeLabel.y() + gBarToLabelOffsetY);

      m_scoreLabel.setPos(m_timeLabel.x(),
                          m_timeLabel.y() + gEffectiveInfoFontSize + (gGuiSpacer / 2.0));
      m_scoreCounter.setPos(m_scoreLabel.x() + gDistBarkPowerLabelToBar,
                            m_scoreLabel.y());
    }
  }
  else
  {
    //!!! fix calculations for non-touchscreen: bars on same height + score below

    m_infosBackground.setPos(0.0, yOffset);

    gDistBarkPowerLabelToBar = m_barkPowerLabel.document()->size().width() + gGuiSpacer;

    m_barkPowerLabel.setPos(gGuiSpacer / 2.0, (gGuiSpacer / 2.0) + yOffset);
    m_barkPowerBar.setPos(m_barkPowerLabel.x() + gDistBarkPowerLabelToBar,
                          m_barkPowerLabel.y() + gBarToLabelOffsetY);

    m_timeLabel.setPos(gGuiSpacer + gDistBarkPowerLabelToBar + gInfoBarLength + gGuiSpacer,
                       m_barkPowerLabel.y());
    m_timeBar.setPos(m_timeLabel.x() + gDistTimeLabelToBar,
                     m_barkPowerBar.y());

    m_scoreLabel.setPos(m_barkPowerLabel.x(), m_barkPowerLabel.y() + (3.0 * gGuiSpacer));
    m_scoreCounter.setPos(m_scoreLabel.x() + gDistBarkPowerLabelToBar,
                          m_scoreLabel.y());
  }

  gInactiveBackground->setPos(0.0, 0.0);
}

void InfoItems::updateBarkPowerBar()
{
  if(gVerticalBars)
  {
    m_barkPowerBar.setRect(0, 0,
                           gInfoBarWidth,
                           (((Dog*)(gDogList.at(0)))->m_barkPower / kMaxBarkPower) * gInfoBarLength);
  }
  else
  {
    m_barkPowerBar.setRect(0, 0,
                           (((Dog*)(gDogList.at(0)))->m_barkPower / kMaxBarkPower) * gInfoBarLength,
                           gInfoBarWidth);
  }
}

void InfoItems::updateTime(long unsigned int timeLeft, long unsigned int totalTime, long unsigned int bonusTime)
{
  if(gVerticalBars)
    m_timeBar.setRect(0, 0,
                      gInfoBarWidth, ((qreal)timeLeft / (qreal)totalTime) * gInfoBarLength);
  else
    m_timeBar.setRect(0, 0,
                      ((qreal)timeLeft / (qreal)totalTime) * gInfoBarLength, gInfoBarWidth);

  if(bonusTime == 0)
  {
    m_bonusTimeBar.setVisible(false);
  }
  else
  {
    m_bonusTimeBar.setVisible(true);
    if(gVerticalBars)
    {
      m_bonusTimeBar.setRect(0, 0,
                             gInfoBarWidth, ((qreal)bonusTime / (qreal)totalTime) * gInfoBarLength);
      m_bonusTimeBar.setPos(m_timeBar.x(), m_timeBar.y() + m_timeBar.rect().height());
    }
    else
    {
      m_bonusTimeBar.setRect(0, 0,
                             ((qreal)bonusTime / (qreal)totalTime) * gInfoBarLength, gInfoBarWidth);
      m_bonusTimeBar.setPos(m_timeBar.x() + m_timeBar.rect().width(), m_timeBar.y());
    }
  }
}

void InfoItems::updateScore()
{
  m_scoreCounter.setPlainText(QString("%1").arg(gDogScore));
}
