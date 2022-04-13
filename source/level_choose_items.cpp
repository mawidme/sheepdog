#include "level_choose_items.hpp"
#include "globals.hpp"
#include "utils.hpp"
#include "mygraphicsscene.hpp"

#include <QTextDocument>
#include <QPen>

QColor kBackColor(100, 100, 100, 180);
QColor kFrontColor(60, 60, 60, 180);


// ========================================================================================
// SINGLE ITEM

LevelChooseItem::LevelChooseItem(const long int number, qreal width)
{
  QFont font;
  font.setBold(false);
  font.setPixelSize(36.0);

  m_number = number;
  QString text = QString("%1").arg(m_number);
  m_text.setDefaultTextColor(kFrontColor);
  m_text.setFont(font);
  m_text.setPlainText(text);
  m_text.setPos(gGuiSpacer / 2.0, gGuiSpacer / 2.0);
  m_text.setParentItem(this);

  m_frame.setRect(0.0, 0.0, width, width);
  m_frame.setBrush(kBackColor);
  QPen pen(kFrontColor);
  pen.setWidthF(3.0);
  m_frame.setPen(pen);
  m_frame.setBrush(QBrush());
  m_frame.setPos(0, 0);
  m_frame.setParentItem(this);
}

QRectF LevelChooseItem::boundingRect() const
{
  return m_frame.rect();
}

// Returns true if point is on button (in scene coordinates!)
bool LevelChooseItem::contains(const QPointF &point) const
{
  if(m_frame.rect().translated(scenePos()).contains(point))
    return true;

  return false;
}

long int LevelChooseItem::number()
{
  return m_number;
}


// ========================================================================================
// ALL ITEMS

static const qreal kDefaultFrameWidth = 54.0;

LevelChooseItems::LevelChooseItems() : m_source(InvalidSource), p_leaveItem(NULL), m_initialized(false)
{
}

LevelChooseItems::~LevelChooseItems()
{
  clearList();

  if(p_leaveItem != NULL)
  {
    delete p_leaveItem;
    p_leaveItem = NULL;
  }

  m_initialized = false;
}

void LevelChooseItems::clearList()
{
  while(!m_levelItemList.empty())
  {
    LevelChooseItem *pItem = m_levelItemList.front();
    m_levelItemList.pop_front();
    delete pItem;
  }
}

QRectF LevelChooseItems::boundingRect() const
{
  return QRectF(pos().x(), pos().y(), gWindowWidth, gWindowHeight);
}

void LevelChooseItems::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{

}

bool LevelChooseItems::init(const LevelSource source, bool rescanLevels)
{
  if(source != BuiltInSource && source != UserSource)
  {
    qDebug("invalid level source %d!", source);
    return false;
  }

  if(m_initialized)
  {
    // rescan level files if demanded or source changed
    if(rescanLevels || source != m_source)
      clearList();
    else
      return true;
  }

  m_source = source;

  int numLevels = 0;
  QString filename;
  while(1)
  {
    filename.clear();
    if(m_source == BuiltInSource)
      filename = QString(GET_FILE("levels/level%1.xml")).arg(numLevels+1);
    else if(m_source == UserSource)
      filename = QString(GET_USER_LEVEL_FILE("level%1.xml")).arg(numLevels+1);

    if(checkFileExists(filename, false))
      numLevels++;
    else
      break;
  }

  if(numLevels == 0)
  {
    /*!!! this doesn't work as message box object is already in use!
    QList<QString> stringList;
    stringList.append(QString("return"));
    if(m_source == BuiltInSource)
      ((MyGraphicsScene*)gScene)->showMessageBox(QString("no levels found!"), &stringList, NULL, NULL);
    else if(m_source == SdSource)
      ((MyGraphicsScene*)gScene)->showMessageBox(QString("no user levels found!"), &stringList, NULL, NULL);
    */
    return false;
  }

  m_frame.setRect(0.0, 0.0, gWindowWidth-6.0, gWindowHeight-6.0);
  m_frame.setBrush(kBackColor);
  QPen pen(kFrontColor);
  pen.setWidthF(3.0);
  m_frame.setPen(pen);
  m_frame.setBrush(QBrush());
  m_frame.setPos(0, 0);
  m_frame.setParentItem(this);

  m_itemWidth = kDefaultFrameWidth * gScreenShrinkFactor;

  qreal itemSpacer = gGuiSpacer;

  qreal posX = itemSpacer;
  qreal posY = itemSpacer;
  for(int number=1; number<=numLevels; number++)
  {
    if(posX >= m_frame.boundingRect().width() - m_itemWidth - gGuiSpacer)
    {
      posX = itemSpacer;
      posY += m_itemWidth + itemSpacer;
    }

    // TODO: handle case where not all levels fit on one page!
    //if(posY >= ...)

    LevelChooseItem *pItem = new LevelChooseItem(number, m_itemWidth);
    pItem->setPos(QPointF(posX, posY));
    pItem->setParentItem(this);
    m_levelItemList.append(pItem);

    posX += m_itemWidth + itemSpacer;
  }

  if(p_leaveItem == NULL)
    p_leaveItem = loadSvgImage("img/editor/leave.svg");

  p_leaveItem->setPos(m_frame.boundingRect().width() - p_leaveItem->boundingRect().width() - gGuiSpacer,
                      m_frame.boundingRect().height() - p_leaveItem->boundingRect().height() - gGuiSpacer);
  if(gScreenShrinkFactor != 1.0)
    p_leaveItem->setScale(gScreenShrinkFactor);
  p_leaveItem->setParentItem(this);

  m_initialized = true;

  return true;
}

int LevelChooseItems::checkClick(QPointF scenePoint)
{
  foreach(LevelChooseItem *pItem, m_levelItemList)
  {
    if(pItem->contains(scenePoint))
      return pItem->number();
  }

  // return 0 if leave button is clicked
  if(p_leaveItem->boundingRect().translated(p_leaveItem->scenePos()).contains(scenePoint))
    return 0;

  return -1;
}
