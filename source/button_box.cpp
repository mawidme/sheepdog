#include "button_box.hpp"

#include "utils.hpp"

#include <QtGui/QTextDocument>

// ===========================================================================
// BUTTON
void MyButton::init(const QString &text)
{
  QFont font;
  font.setBold(false);
  font.setPixelSize(36.0);

  m_buttonTextItem.setDefaultTextColor(kInfoColor);
  m_buttonTextItem.setFont(font);
  m_buttonTextItem.setPlainText(text);
  m_buttonTextItem.setPos(gGuiSpacer / 2.0, gGuiSpacer / 2.0);
  m_buttonTextItem.setParentItem(this);

  QSizeF textSize = m_buttonTextItem.document()->size();
  m_buttonFrameItem.setRect(0.0, 0.0,
                            gGuiSpacer + textSize.width(), gGuiSpacer + textSize.height());
  QPen pen(kInfoColor);
  pen.setWidthF(3.0);
  m_buttonFrameItem.setPen(pen);
  m_buttonFrameItem.setBrush(QBrush());
  m_buttonFrameItem.setPos(0, 0);
  m_buttonFrameItem.setParentItem(this);
}

qreal MyButton::width()
{
  if(m_buttonFrameItem.rect().isNull())
    return 0;

  return m_buttonFrameItem.rect().width();
}

qreal MyButton::height()
{
  if(m_buttonFrameItem.rect().isNull())
    return 0;

  return m_buttonFrameItem.rect().height();
}

QRectF MyButton::boundingRect() const
{
  return m_buttonFrameItem.rect();
}

// Returns true if point is on button (in scene coordinates!)
bool MyButton::contains(const QPointF &point) const
{
  // return false if button is not initialized
  if(m_buttonFrameItem.rect().isNull())
    return false;

  if(m_buttonFrameItem.rect().translated(scenePos()).contains(point))
    return true;

  return false;
}

void MyButton::clear()
{
  m_buttonFrameItem.setRect(QRectF(0, 0, 0, 0));
  m_buttonTextItem.setPlainText(QString(""));
}


// ===========================================================================
// BUTTON BOX
MyButtonBox::MyButtonBox() : m_initialized(false), p_boxFrameItem(NULL)
{
  setVisible(false);
}

bool MyButtonBox::init(const QString &text, const QList<QString> *pButtonTextList,
                       void (*pButtonHandler)(ButtonContext*), void *pContext)
{
  if(pButtonTextList->empty())
  {
    qDebug("no button string given!");
    return false;
  }

  if(gShowingMsgBox)
  {
    qDebug("already showing a message box!");
    return false;
  }

  gShowingMsgBox = true;

  // 1) reset stuff
  m_buttonContext.buttonNumber = 0;
  m_buttonContext.p_handlerFunction = pButtonHandler;
  m_buttonContext.p_context = pContext;

  // 2.a) add main frame
  if(p_boxFrameItem == NULL)
    if((p_boxFrameItem = loadSvgImage("img/boxBackground.svg")) == NULL)
      return false;
  p_boxFrameItem->setPos(0, 0);
  p_boxFrameItem->setParentItem(this);

  // scale factor etc.
  p_boxFrameItem->setScale(1.0);

  // Distance from frame to text resp. buttons.
  qreal frameDist = 5*gGuiSpacer;

  // create buttons
  MyButton *pButton;
  foreach(QString buttonText, *pButtonTextList)
  {
    pButton = new MyButton();
    pButton->init(buttonText);
    pButton->setParentItem(this);
    m_buttonList.append(pButton);
  }

  if(text.isEmpty())
  {
    // if no main text is given, align buttons vertically

    m_boxTextItem.setVisible(false);

    qreal buttonsTotalWidth = 2*gGuiSpacer;
    qreal buttonsTotalHeight = 2*gGuiSpacer;

    foreach(MyButton *pButton, m_buttonList)
    {
      buttonsTotalWidth += pButton->width() + 2*gGuiSpacer;
      buttonsTotalHeight += pButton->height() + 2*gGuiSpacer;
    }

    qreal scale = qMax(buttonsTotalWidth / width(), buttonsTotalHeight / height());
    p_boxFrameItem->setScale(scale);

    qreal posY = 2*gGuiSpacer;
    foreach(MyButton *pButton, m_buttonList)
    {
      pButton->setPos((p_boxFrameItem->boundingRect().width()*scale - pButton->width()) / 2, posY);
      posY += pButton->height() + 2*gGuiSpacer;
    }
  }
  else
  {
    // if main text is given, show text on top and align buttons horizontally below it

    m_boxTextItem.setVisible(true);

    // 2.b2) add main text
    m_boxTextItem.setDefaultTextColor(kInfoColor);
    m_boxTextItem.setFont(gInfoFont);
    m_boxTextItem.setPlainText(text);
    m_boxTextItem.setPos(2*gGuiSpacer, frameDist);
    m_boxTextItem.setParentItem(this);
    QSizeF textSize = m_boxTextItem.document()->size();

    // 2.c) add buttons
    qreal posX = 2*gGuiSpacer;
    qreal posY = m_boxTextItem.y() + textSize.height() + gGuiSpacer;
    foreach(MyButton *pButton, m_buttonList)
    {
      pButton->setPos(posX, posY);
      posX += pButton->width() + gGuiSpacer;
    }

    // TODO: random generator for frame?

    // calculate width necessary to show buttons
    qreal buttonsTotalWidth = 0;
    foreach(MyButton *pButton, m_buttonList)
      buttonsTotalWidth += pButton->width();

    // 3) adjust frame to text size and re-position:

    // main text field or buttons may be wider. decide here without taking distance to frame into account.
    qreal elementsWidth = qMax(buttonsTotalWidth + (m_buttonList.size() - 1) * gGuiSpacer,
                               textSize.width());
    qreal scaleX = elementsWidth / (width() - 2*frameDist);

    qreal elementsHeight = m_buttonList.at(0)->y() + m_buttonList[0]->height() - frameDist;
    qreal scaleY = elementsHeight / (height() - 2*frameDist);

    if(scaleX > scaleY)
    {
      // 3.a1) scale frame
      p_boxFrameItem->setScale(scaleX);

      // 3.b) main text and buttons; vertically
      qreal elmtVertDist = (height() - 6 * gGuiSpacer - elementsHeight) / 3.0;
      qreal posY = 3 * gGuiSpacer + elmtVertDist;
      m_boxTextItem.setPos(m_boxTextItem.x(), posY);
      posY += textSize.height() + elmtVertDist;
      foreach(MyButton *pButton, m_buttonList)
        pButton->setPos(pButton->x(), posY);
    }
    else
    {
      // 3.a2) scale frame
      p_boxFrameItem->setScale(scaleY);
    }

    // 3.c) adjust text horizontally (done in any case as buttons may be wider than text)
    qreal dx = (width() - textSize.width()) / 2.0;
    m_boxTextItem.moveBy(dx, 0);

    // 3.d) adjust buttons horizontally after scaling:
    qreal buttonHorDist = (width() - 6 * gGuiSpacer - elementsWidth) / (m_buttonList.size() + 1);
    posX = 3 * gGuiSpacer + buttonHorDist;
    foreach(MyButton *pButton, m_buttonList)
    {
      pButton->setPos(posX, pButton->y());
      posX += pButton->width() + buttonHorDist;
    }
  }

  m_initialized = true;
  setVisible(true);

  return true;
}

QRectF MyButtonBox::boundingRect() const
{
  return p_boxFrameItem->boundingRect();
}

bool MyButtonBox::checkHit(const QPointF &sceneClickPos)
{
  int buttonNumber = 0;
  int i = 1;
  foreach(MyButton *pButton, m_buttonList)
  {
    if(pButton->contains(sceneClickPos))
    {
      buttonNumber = i;
      break;
    }

    i++;
  }

  if(buttonNumber != 0)
  {
    qDebug("box button %d clicked", buttonNumber);

    // 1) copy button context if needed
    ButtonContext contextCopy;
    memset(&contextCopy, 0x00, sizeof(ButtonContext));
    if(m_buttonContext.p_handlerFunction != NULL)
    {
      m_buttonContext.buttonNumber = buttonNumber;
      memcpy(&contextCopy, &m_buttonContext, sizeof(ButtonContext));
    }

    // 2) reset everything
    clear();

    // 3) call button callback function if any
    if(contextCopy.p_handlerFunction != NULL)
      contextCopy.p_handlerFunction(&contextCopy);

    return true;
  }

  return false;
}

void MyButtonBox::clear()
{
  setVisible(false);
  gShowingMsgBox = false;
  m_initialized = false;
  m_buttonContext.buttonNumber = 0;
  m_buttonContext.p_handlerFunction = NULL;
  m_buttonContext.p_context = NULL;

  while(!m_buttonList.isEmpty())
  {
    MyButton *pButton = m_buttonList.front();
    m_buttonList.pop_front();
    delete pButton;
  }
  m_boxTextItem.setPlainText(QString(""));
}

qreal MyButtonBox::width()
{
  return p_boxFrameItem->boundingRect().width() * p_boxFrameItem->scale();
}

qreal MyButtonBox::height()
{
  return p_boxFrameItem->boundingRect().height() * p_boxFrameItem->scale();
}
