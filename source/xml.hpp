#ifndef XML_HPP_
#define XML_HPP_

#include "level_types.hpp"

#include <QtXml/QXmlDefaultHandler>
#include <QtXml/QXmlAttributes>

class LevelSettings
{
public:
  // level dimensions in number of tiles
  long unsigned int m_width;
  long unsigned int m_height;

  // time available for level, in seconds
  long unsigned int m_time;
};

class Xml : public QXmlDefaultHandler
{
public:
  Xml();

  bool parse(const LevelSource levelSource, unsigned long int levelNumber, LevelSettings *pLevelSettings);
  bool startDocument();
  bool startElement( const QString&, const QString&, const QString &name, const QXmlAttributes &attrs );
  bool endElement( const QString&, const QString&, const QString &name );

  int saveLevelXmlFile(const LevelSettings *pLevelSettings, long unsigned int *pLevelNumber);
  void writeStartDocument();

private:
  LevelSettings *p_levelSettings;

  bool m_inLevel;
  bool m_inSettings;
  bool m_inTiles;
  bool m_inCharacters;
};

#endif /* XML_HPP_ */
