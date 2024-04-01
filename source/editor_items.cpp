#include "editor_items.hpp"
#include "globals.hpp"
#include "utils.hpp"

#include <QtGui/QTextDocument>

static const char *kEditorImageNames[NumEditTypes] = { "img/editor/fence.svg",
                                                       "img/editor/bush.svg",
                                                       "img/editor/tree.svg",
                                                       "img/editor/water.svg",
                                                       "img/editor/exit.svg",
                                                       "img/editor/dog.svg",
                                                       "img/editor/sheep.svg",
                                                       "img/editor/undo.svg",
                                                       "img/editor/eraser.svg",
                                                       "img/editor/pan.svg",
                                                       "img/editor/load.svg",
                                                       "img/editor/save.svg",
                                                       "img/editor/play.svg",
                                                       "img/editor/leave.svg"};
static const int kFirstRightEditTool = (int)UndoEdit;

static const qreal kDefaultToolImageWidth = 54.0;

static const qreal kRectPenWidth = 2.0;

EditorItems::EditorItems() : m_initialized(false)
{
  for(int i=0; i<NumEditTypes; i++)
    p_editTools[i] = NULL;
}

EditorItems::~EditorItems()
{
  for(int i=0; i<NumEditTypes; i++)
  {
    if(p_editTools[i] != NULL)
    {
      delete p_editTools[i];
      p_editTools[i] = NULL;
    }
  }
}

QRectF EditorItems::boundingRect() const
{
  return QRectF(pos().x(), pos().y(), gWindowWidth, gWindowHeight);
}

void EditorItems::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{

}

void EditorItems::init()
{
  if(m_initialized)
    return;

  m_toolImageWidth = kDefaultToolImageWidth * gScreenShrinkFactor;

  qreal toolSpacer = gGuiSpacer;

  QPen pen;
  qreal posX = toolSpacer;
  qreal posY = toolSpacer;
  for(int i=0; i<NumEditTypes; i++)
  {
    if(i == kFirstRightEditTool)
    {
      posX = gWindowWidth - m_toolImageWidth - toolSpacer;
      posY = toolSpacer;
    }

    m_editRectItem[i].setParentItem(this);
    m_editRectItem[i].setPos(posX, posY);
    m_editRectItem[i].setRect(0, 0, m_toolImageWidth, m_toolImageWidth);
    m_editRectItem[i].setBrush(QColor(100, 100, 100, 180));
    pen.setColor(QColor(60, 60, 60, 180));
    pen.setWidthF(kRectPenWidth);
    m_editRectItem[i].setPen(pen);

    m_editRect[i] = m_editRectItem[i].boundingRect();
    m_editRect[i].moveTo(m_editRectItem[i].scenePos());

    posY += m_toolImageWidth + toolSpacer;
  }

  for(int i=0; i<NumEditTypes; i++)
  {
    if(p_editTools[i] == NULL)
      p_editTools[i] = loadSvgImage(kEditorImageNames[i]);

    p_editTools[i]->setParentItem(this);
    p_editTools[i]->setPos(m_editRectItem[i].x() + (m_toolImageWidth - p_editTools[i]->boundingRect().width()) / 2,
                           m_editRectItem[i].y() + (m_toolImageWidth - p_editTools[i]->boundingRect().height()) / 2);

    if(gScreenShrinkFactor != 1.0)
      p_editTools[i]->setScale(gScreenShrinkFactor);
  }

  m_selectedToolRect.setParentItem(this);
  m_selectedToolRect.setPos(m_editRectItem[gEditMode].pos());
  m_selectedToolRect.setRect(m_editRectItem[gEditMode].boundingRect());
  m_selectedToolRect.setBrush(QColor(227, 224, 61, 100));
  pen.setColor(QColor(165, 163, 47, 150));
  pen.setWidthF(kRectPenWidth);
  m_selectedToolRect.setPen(pen);

  m_initialized = true;
}

int EditorItems::checkClick(QPointF scenePoint)
{
  for(int i=0; i<NumEditTypes; i++)
  {
    if(m_editRect[i].contains(scenePoint))
    {
      if(i != (int)SaveEdit &&
         i != (int)UndoEdit &&
         i != (int)PlayEdit &&
         i != (int)LoadEdit &&
         i != (int)LeaveEdit)
      {
        gEditMode = (EditType)i;
        m_selectedToolRect.setPos(m_editRectItem[gEditMode].pos());
      }
      return i;
    }
  }

  return -1;
}
