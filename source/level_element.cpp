#include "level_element.hpp"

#include "utils.hpp"

LevelElement::LevelElement() : p_pngImg(NULL),
                               p_svgImg(NULL),
                               m_elemType(UnknownLevelElement),
                               p_colRect(NULL)
{
  m_imgType = UnknownImgType;
}

LevelElement::~LevelElement()
{
  if(p_colRect != NULL)
    delete p_colRect;

  switch(m_imgType)
  {
  case PngImgType:
    if(p_pngImg != NULL)
      delete p_pngImg;
    break;

  case PngImgListType:
    while(!m_pngImgList.isEmpty())
    {
      QGraphicsPixmapItem *p_imgItem = m_pngImgList.front();
      m_pngImgList.pop_front();
      delete p_imgItem;
    }
    break;

  case SvgImgType:
    if(p_svgImg != NULL)
      delete p_svgImg;
    break;

  case CustomImgType:
    // Nothing to do here, element should clean up itself (specific destructor)
    break;

  default:
    qDebug("Invalid image type %d!", m_imgType);
    break;
  }
}

// Returns a non-zero size if successful
QSizeF LevelElement::loadElmtGraphics(long unsigned int sx, long unsigned int sy)
{
  QSizeF size(0, 0);
  QString tmpFileName;

  // Use vector graphics if device is fast enough (only if image type not set before!)
  if(m_imgType == UnknownImgType)
  {
    if(gGraphicQuality >= VeryHighQuality)
      m_imgType = SvgImgType;
    else
      m_imgType = PngImgType;
  }

  if(m_imgType == UnknownImgType)
  {
    qDebug("Cannot load graphics for unknown level element!");
    goto finish;
  }
  if(m_imgFileName == "")
  {
    qDebug("Cannot load graphics for empty file name!");
    goto finish;
  }

  // file prefix ":/" will be added in loadPngImage() resp. loadSvgImage()
  tmpFileName = QString("img/%1.%2").arg(m_imgFileName).arg(getImageSuffix());

  if(m_imgType == PngImgType)
  {
    if(gPngLevelImg[0] == NULL)
    {
      for(int iElmt=0; iElmt<NumLevelElemTypes; iElmt++)
      {
        gPngLevelImg[iElmt] = new QPixmap*[gNumLevelElmtStyles[iElmt]];
        for(int style=0; style<gNumLevelElmtStyles[iElmt]; style++)
          gPngLevelImg[iElmt][style] = NULL;
      }
    }

    if(gPngLevelImg[m_elemType][m_imgStyle] == NULL)
      gPngLevelImg[m_elemType][m_imgStyle] = loadPngImage(tmpFileName);

    p_pngImg = new QGraphicsPixmapItem(*gPngLevelImg[m_elemType][m_imgStyle]);
    p_pngImg->setParentItem(this);

    initColRect(p_pngImg->boundingRect().translated(sx, sy));

    size = p_pngImg->boundingRect().size();
  }
  else if(m_imgType == SvgImgType)
  {
    if(gSvgLevelRenderer[0] == NULL)
    {
      for(int iElmt=0; iElmt<NumLevelElemTypes; iElmt++)
      {
        gSvgLevelRenderer[iElmt] = new QSvgRenderer*[gNumLevelElmtStyles[iElmt]];
        for(int style=0; style<gNumLevelElmtStyles[iElmt]; style++)
          gSvgLevelRenderer[iElmt][style] = NULL;
      }
    }

    if(gSvgLevelRenderer[m_elemType][m_imgStyle] == NULL)
      loadSvgImage(&gSvgLevelRenderer[m_elemType][m_imgStyle], tmpFileName);

    p_svgImg = new QGraphicsSvgItem();
    p_svgImg->setSharedRenderer(gSvgLevelRenderer[m_elemType][m_imgStyle]);
    p_svgImg->setParentItem(this);

    initColRect(p_svgImg->boundingRect().translated(sx, sy));

    size = p_svgImg->boundingRect().size();
  }
  else
  {
    qDebug("Image type %d not handled here!", m_imgType);
    goto finish;
  }

  setPos(sx, sy);

  m_imgInit = true;

finish:
  return size;
}

void LevelElement::initColRect(QRectF rect)
{
  initColRect(rect.x(), rect.y(), rect.width(), rect.height());
}

void LevelElement::initColRect(qreal x, qreal y, qreal width, qreal height)
{
  p_colRect = new QRectF(x, y, width, height);

#ifdef SHOW_COL_RECTS
  m_colRectItem.setRect(*m_colRect);
  m_colRectItem.setPen(kInactiveColRectPen);
  m_colRectItem.setBrush(kInactiveColRectBrush);
#endif
}

LevelElemType LevelElement::getElemType()
{
  return m_elemType;
}

QRectF* LevelElement::getCollisionRect()
{
  return p_colRect;
}


// ==============================================================================================
// implementations of virtual QGraphicsItem methods
// ==============================================================================================

void LevelElement::paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget)
{
}

QRectF LevelElement::boundingRect() const
{
  return m_boundingRect;
}

void LevelElement::advance(int step)
{
}

void LevelElement::moveByDelta(qreal dx, qreal dy)
{
  moveBy(dx, dy);
  p_colRect->translate(dx, dy);
  m_topLeftPos += QPointF(dx, dy);
  m_bottomRightPos += QPointF(dx, dy);
}

void LevelElement::moveByDeltaPoint(QPointF d)
{
  moveBy(d.x(), d.y());
  p_colRect->translate(d.x(), d.y());
  m_topLeftPos += d;
  m_bottomRightPos += d;
}
