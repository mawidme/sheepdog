#include "xml.hpp"

#include "global_lists.hpp"

#include "exit.hpp"
#include "fence.hpp"
#include "water.hpp"
#include "bush.hpp"
#include "tree.hpp"
#include "deco.hpp"

#include <QtCore/QXmlStreamWriter>
#include <QtCore/QDir>
#include <QtWidgets/QMessageBox>

Xml::Xml()
{
  m_inLevel = false;
  m_inSettings = false;
  m_inTiles = false;
  m_inCharacters = false;
}

bool Xml::parse(const LevelSource levelSource, unsigned long int levelNumber, LevelSettings *pLevelSettings)
{
  if(pLevelSettings == NULL)
  {
    qDebug("level settings NULL!");
    return false;
  }

  p_levelSettings = pLevelSettings;

  QString filename;
  if(levelSource == BuiltInSource)
    filename = QString(GET_FILE("levels/level%1.xml")).arg(levelNumber);
  else if(levelSource == UserSource)
    filename = QString(GET_USER_LEVEL_FILE("level%1.xml")).arg(levelNumber);
  else
  {
    qDebug("invalid level source %d!", levelSource);
    return false;
  }

  if(!checkFileExists(filename))
    return false;

  QFile file(filename);
  QXmlInputSource source(&file);
  QXmlSimpleReader reader;
  reader.setContentHandler(this);

  if(!reader.parse(source))
    return false;

  return true;
}

bool Xml::startDocument()
{
  m_inLevel = false;
  return true;
}

bool Xml::startElement( const QString&, const QString&, const QString &name, const QXmlAttributes &attrs )
{
  if( name == "level" )
  {
    m_inLevel = true;
    return true;
  }
  else if( name == "settings" )
  {
    m_inSettings = true;
    return true;
  }
  else if( name == "tiles" )
  {
    m_inTiles = true;
    return true;
  }
  else if( name == "characters" )
  {
    m_inCharacters = true;
    return true;
  }

  if(!m_inLevel)
  {
      qDebug("Ignored \"%s\": Not in level!", name.toStdString().c_str());
      return true;
  }

  if(m_inSettings)
  {
    if( name == "size" )
    {
      if(attrs.count() != 2)
      {
        qDebug("Invalid number of size attribute: %d", attrs.count());
        return false;
      }

      for( int i=0; i<attrs.count(); i++ )
      {
        if( attrs.localName( i ) == "width" )
          p_levelSettings->m_width = attrs.value( i ).toULong();
        else if( attrs.localName( i ) == "height" )
          p_levelSettings->m_height = attrs.value( i ).toULong();
        else
        {
          qDebug("Unknown size attribute: %s", attrs.localName(i).toStdString().c_str());
          return false;
        }
      }
    }
    else if( name == "time" )
    {
      if(attrs.count() != 1)
      {
        qDebug("Invalid number of time attribute: %d", attrs.count());
        return false;
      }
      for( int i=0; i<attrs.count(); i++ )
      {
        if( attrs.localName( i ) == "seconds" )
        {
          p_levelSettings->m_time = attrs.value( i ).toULong();
        }
        else
        {
          qDebug("Unknown time attribute: %s", attrs.localName( i ).toStdString().c_str());
          return false;
        }
      }
    }
    else
    {
      qDebug("Ignored '%s' in settings!", name.toStdString().c_str());
      return true;
    }
  }
  else if(m_inTiles)
  {
    if( name == "fence" )
    {
      if(attrs.count() != 4)
      {
        qDebug("Invalid number of fence attributes: %d", attrs.count());
        return false;
      }

      long unsigned int xStart = kInvalidUint;
      long unsigned int yStart = kInvalidUint;
      long unsigned int xEnd = kInvalidUint;
      long unsigned int yEnd = kInvalidUint;
      for( int i=0; i<attrs.count(); i++ )
      {
        if( attrs.localName( i ) == "xStart" )
          xStart = attrs.value( i ).toULong();
        else if( attrs.localName( i ) == "yStart" )
          yStart = attrs.value( i ).toULong();
        else if( attrs.localName( i ) == "xEnd" )
          xEnd = attrs.value( i ).toULong();
        else if( attrs.localName( i ) == "yEnd" )
          yEnd = attrs.value( i ).toULong();
        else
        {
          qDebug("Unknown hor fence attribute: %s", attrs.localName( i ).toStdString().c_str());
          return false;
        }
      }

      if(xStart == kInvalidUint || yStart == kInvalidUint ||
         xEnd == kInvalidUint || yEnd == kInvalidUint)
      {
        qDebug("XML attributes for vert. fence are incomplete!");
        return false;
      }
      Fence *horFence = new Fence(xStart, yStart, xEnd, yEnd);
      gLevelElementList.append(horFence);
    }
    else if( name == "water" )
    {
      if(attrs.count() != 5)
      {
        qDebug("Invalid number of water attributes: %d", attrs.count());
        return false;
      }

      long unsigned int xStart = kInvalidUint;
      long unsigned int yStart = kInvalidUint;
      long unsigned int xEnd = kInvalidUint;
      long unsigned int yEnd = kInvalidUint;
      long unsigned int style = kInvalidUint;
      for( int i=0; i<attrs.count(); i++ )
      {
        if( attrs.localName( i ) == "xStart" )
          xStart = attrs.value( i ).toULong();
        else if( attrs.localName( i ) == "yStart" )
          yStart = attrs.value( i ).toULong();
        else if( attrs.localName( i ) == "xEnd" )
          xEnd = attrs.value( i ).toULong();
        else if( attrs.localName( i ) == "yEnd" )
          yEnd = attrs.value( i ).toULong();
        else if( attrs.localName( i ) == "style" )
          style = attrs.value( i ).toULong();
        else
        {
          qDebug("Unknown water attribute: %s", attrs.localName( i ).toStdString().c_str());
          return false;
        }
      }

      if(xStart == kInvalidUint || yStart == kInvalidUint || xEnd == kInvalidUint || yEnd == kInvalidUint || style == kInvalidUint)
      {
        qDebug("XML attributes for water are incomplete!");
        return false;
      }
      Water *water = new Water(QPointF(xStart, yStart), QPointF(xEnd, yEnd), style);
      gLevelElementList.append(water);
    }
    else if(name == "bush" ||
            name == "tree" ||
            name == "exit")
    {
      if(attrs.count() != 3)
      {
        qDebug("Invalid number of bush/tree/exitattributes: %d", attrs.count());
        return false;
      }

      bool isBush = false;
      bool isTree = false;
      bool isExit = false;
      if(name == "bush")
        isBush = true;
      else if(name == "tree")
        isTree = true;
      else if(name == "exit")
        isExit = true;

      long unsigned int x = kInvalidUint;
      long unsigned int y = kInvalidUint;
      long unsigned int style = kInvalidUint;
      for( int i=0; i<attrs.count(); i++ )
      {
        if( attrs.localName( i ) == "x" )
          x = attrs.value( i ).toDouble();
        else if( attrs.localName( i ) == "y" )
          y = attrs.value( i ).toDouble();
        else if( attrs.localName( i ) == "style" )
          style = attrs.value( i ).toDouble();
        else
        {
          qDebug("Unknown attribute: %s", attrs.localName( i ).toStdString().c_str());
          return false;
        }
      }

      if(x == kInvalidUint || y == kInvalidUint || style == kInvalidUint)
      {
        qDebug("XML attributes are incomplete!");
        return false;
      }

      if(isBush)
      {
        Bush *bush = new Bush(x, y, style);
        gLevelElementList.append(bush);
      }
      else if(isExit)
      {
        ExitElem *exitElem = new ExitElem(x, y, style);
        gLevelElementList.append(exitElem);
      }
      else if(isTree)
      {
        Tree *tree = new Tree(x, y, style);
        gLevelElementList.append(tree);
      }
    }
    else
    {
      qDebug("Ignored '%s' in tiles!", name.toStdString().c_str());
      return true;
    }
  }
  else if(m_inCharacters)
  {
    if( name == "dog" )
    {
      if(attrs.count() != 2)
      {
        qDebug("Invalid number of dog attribute: %d", attrs.count());
        return false;
      }
      long unsigned int x = kInvalidUint;
      long unsigned int y = kInvalidUint;
      for( int i=0; i<attrs.count(); i++ )
      {
        if( attrs.localName( i ) == "x" )
          x = attrs.value( i ).toULong();
        else if( attrs.localName( i ) == "y" )
          y = attrs.value( i ).toULong();
        else
        {
          qDebug("Unknown dog attribute: %s", attrs.localName( i ).toStdString().c_str());
          return false;
        }
      }

      if(x == kInvalidUint || y == kInvalidUint)
      {
        qDebug("XML attributes for dog are incomplete!");
        return false;
      }
      Dog *p_dog = new Dog(x, y);
      gDogList.append((Character*)p_dog);
    }
    else if( name == "sheep" )
    {
      if(attrs.count() != 2)
      {
        qDebug("Invalid number of sheep attribute: %d", attrs.count());
        return false;
      }
      long unsigned int x = kInvalidUint;
      long unsigned int y = kInvalidUint;
      for( int i=0; i<attrs.count(); i++ )
      {
        if( attrs.localName( i ) == "x" )
          x = attrs.value( i ).toULong();
        else if( attrs.localName( i ) == "y" )
          y = attrs.value( i ).toULong();
        else
        {
          qDebug("Unknown sheep attribute: %s", attrs.localName( i ).toStdString().c_str());
          return false;
        }
      }

      if(x == kInvalidUint || y == kInvalidUint)
      {
        qDebug("XML attributes for dog are incomplete!");
        return false;
      }
      Sheep *sheep = new Sheep(x, y);
      gSheepList.append(sheep);
    }
    else
    {
      qDebug("Ignored '%s' in characters!", name.toStdString().c_str());
      return true;
    }
  }
  else
  {
    qDebug("Ignored '%s' inside of nowhere!", name.toStdString().c_str());
    return true;
  }

  return true;
}

bool Xml::endElement( const QString&, const QString&, const QString &name )
{
  if( name == "level" )
    m_inLevel = false;
  else if( name == "settings" )
    m_inSettings = false;
  else if( name == "tiles" )
    m_inTiles = false;
  else if( name == "characters" )
    m_inCharacters = false;

  return true;
}

int Xml::saveLevelXmlFile(const LevelSettings *pLevelSettings, long unsigned int *pLevelNumber)
{
  // A) CHECK LEVEL

  bool fenceFound = false;
  bool waterFound = false;
  bool treeFound = false;
  bool bushFound = false;
  foreach(LevelElement *pElement, gLevelElementList)
  {
    if(pElement->getElemType() == FenceLevelElement)
      fenceFound = true;
    else if(pElement->getElemType() == WaterLevelElement)
      waterFound = true;
    else if(pElement->getElemType() == TreeLevelElement)
      treeFound = true;
    else if(pElement->getElemType() == BushLevelElement)
      bushFound = true;
  }

  // B) GENERATE XML FILE CONTENT

  QString outputStr;
  QXmlStreamWriter stream(&outputStr);
  stream.setAutoFormatting(true);
  stream.setAutoFormattingIndent(2);
  stream.writeStartDocument();

  stream.writeStartElement("level");


  // 1) settings
  stream.writeStartElement("settings");
  stream.writeStartElement("size");
  stream.writeAttribute("width", QString("%1").arg(pLevelSettings->m_width));
  stream.writeAttribute("height", QString("%1").arg(pLevelSettings->m_height));
  stream.writeEndElement();
  stream.writeStartElement("time");
  stream.writeAttribute("seconds", QString("%1").arg(pLevelSettings->m_time));
  stream.writeEndElement();
  stream.writeEndElement();


  // 2) tiles
  stream.writeStartElement("tiles");

  // 2.a) fences
  if(fenceFound)
  {
    foreach(LevelElement *pElement, gLevelElementList)
    {
      if(pElement->getElemType() == FenceLevelElement)
      {
        stream.writeStartElement("fence");
        stream.writeAttribute("xStart", QString("%1").arg((int)(pElement->getTopLeftPos().x() + kFenceWidth/2.0)));
        stream.writeAttribute("yStart", QString("%1").arg((int)(pElement->getTopLeftPos().y() + kFenceWidth/2.0)));
        stream.writeAttribute("xEnd", QString("%1").arg((int)(pElement->getBottomRightPos().x() - kFenceWidth/2.0)));
        stream.writeAttribute("yEnd", QString("%1").arg((int)(pElement->getBottomRightPos().y() - kFenceWidth/2.0)));
        stream.writeEndElement();
      }
    }
  }

  // 2.b) bushes
  if(bushFound)
  {
    foreach(LevelElement *pElement, gLevelElementList)
    {
      if(pElement->getElemType() == BushLevelElement)
      {
        stream.writeStartElement("bush");
        stream.writeAttribute("x", QString("%1").arg((int)pElement->getTopLeftPos().x()));
        stream.writeAttribute("y", QString("%1").arg((int)pElement->getTopLeftPos().y()));
        stream.writeAttribute("style", QString("%1").arg(pElement->m_imgStyle + 1));
        stream.writeEndElement();
      }
    }
  }

  // 2.c) trees
  if(treeFound)
  {
    foreach(LevelElement *pElement, gLevelElementList)
    {
      if(pElement->getElemType() == TreeLevelElement)
      {
        stream.writeStartElement("tree");
        stream.writeAttribute("x", QString("%1").arg((int)pElement->getTopLeftPos().x()));
        stream.writeAttribute("y", QString("%1").arg((int)pElement->getTopLeftPos().y()));
        stream.writeAttribute("style", QString("%1").arg(pElement->m_imgStyle + 1));
        stream.writeEndElement();
      }
    }
  }

  // 2.d) water
  if(waterFound)
  {
    foreach(LevelElement *pElement, gLevelElementList)
    {
      if(pElement->getElemType() == WaterLevelElement)
      {
        stream.writeStartElement("water");
        stream.writeAttribute("xStart", QString("%1").arg((int)pElement->getTopLeftPos().x()));
        stream.writeAttribute("yStart", QString("%1").arg((int)pElement->getTopLeftPos().y()));
        stream.writeAttribute("xEnd", QString("%1").arg((int)pElement->getBottomRightPos().x()));
        stream.writeAttribute("yEnd", QString("%1").arg((int)pElement->getBottomRightPos().y()));
        stream.writeAttribute("style", QString("%1").arg(pElement->m_imgStyle + 1));
        stream.writeEndElement();
      }
    }
  }

  // 2.e) exits
  foreach(LevelElement *pElement, gLevelElementList)
  {
    if(pElement->getElemType() == ExitLevelElement)
    {
      stream.writeStartElement("exit");
      stream.writeAttribute("x", QString("%1").arg((int)pElement->getTopLeftPos().x()));
      stream.writeAttribute("y", QString("%1").arg((int)pElement->getTopLeftPos().y()));
      stream.writeAttribute("style", QString("%1").arg(pElement->m_imgStyle + 1));
      stream.writeEndElement();
    }
  }

  // end tiles
  stream.writeEndElement();


  // characters
  stream.writeStartElement("characters");
  foreach(Character *pDog, gDogList)
  {
    stream.writeStartElement("dog");
    stream.writeAttribute("x", QString("%1").arg((int)pDog->absMidPoint().x()));
    stream.writeAttribute("y", QString("%1").arg((int)pDog->absMidPoint().y()));
    stream.writeEndElement();
  }
  foreach(Character *pSheep, gSheepList)
  {
    stream.writeStartElement("sheep");
    stream.writeAttribute("x", QString("%1").arg((int)pSheep->absMidPoint().x()));
    stream.writeAttribute("y", QString("%1").arg((int)pSheep->absMidPoint().y()));
    stream.writeEndElement();
  }
  stream.writeEndElement();

  // end level
  stream.writeEndElement();

  stream.writeEndDocument();

  // create directory if it doesn't exist yet
#ifdef MOBILE_VERSION
  if(!QDir(GET_SD_DIR("sheepdog")).exists())
  {
    QDir dir(GET_SD_DIR(""));
    if(!dir.mkdir("sheepdog"))
    {
      QMessageBox msgBox;
      msgBox.setText(QString("error creating sheepdog dir!"));
      msgBox.exec();
      return DirCreateError;
    }
  }
  if(!QDir(GET_SD_DIR("sheepdog/levels")).exists())
  {
    QDir dir(GET_SD_DIR("sheepdog"));
    if(!dir.mkdir("levels"))
    {
      QMessageBox msgBox;
      msgBox.setText(QString("error creating level dir!"));
      msgBox.exec();
      return DirCreateError;
    }
  }
#else /* MOBILE_VERSION */
  if(!QDir("levels").exists())
  {
    QDir dir("");
    if(!dir.mkdir("levels"))
    {
      QMessageBox msgBox;
      msgBox.setText(QString("error creating level dir!"));
      msgBox.exec();
      return DirCreateError;
    }
  }
  if(!QDir("levels/user").exists())
  {
    QDir dir("levels");
    if(!dir.mkdir("user"))
    {
      QMessageBox msgBox;
      msgBox.setText(QString("error creating user level dir!"));
      msgBox.exec();
      return DirCreateError;
    }
  }
#endif /* MOBILE_VERSION */


  // C) SAVE XML FILE: look for next unused file name if level number not set (e.g. when editing existing level)

  QString filename;
  if(*pLevelNumber == 0)
  {
    for(int i=1; i<1000; i++)
    {
      filename.clear();
      filename = QString(GET_USER_LEVEL_FILE("level%1.xml")).arg(i);
      if(!checkFileExists(filename))
      {
        *pLevelNumber = i;
        break;
      }
    }
  }

  filename = QString(GET_USER_LEVEL_FILE("level%1.xml")).arg(*pLevelNumber);

  QFile file(filename);
  if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
  {
    QMessageBox msgBox;
    msgBox.setText(QString("error opening level xml file!"));
    msgBox.exec();
    return OpenFileError;
  }

  if(file.write(outputStr.toStdString().data(), outputStr.length()) == -1)
  {
    QMessageBox msgBox;
    msgBox.setText(QString("error writing level xml file!"));
    msgBox.exec();
    return WriteFileError;
  }

  return NoError;
}

void Xml::writeStartDocument()
{

}
