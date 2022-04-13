#ifndef LEVEL_TYPES_HPP_
#define LEVEL_TYPES_HPP_

enum CharacterType
{
  DogCharacter,
  SheepCharacter,
  NumCharacterTypes,
  UnknownCharacter
};

enum LevelElemType
{
  DecoLevelElement,
  ExitLevelElement,
  FenceLevelElement,
  WaterLevelElement,
  TreeLevelElement,
  BushLevelElement,
  NumLevelElemTypes,
  UnknownLevelElement
};

enum EditType
{
  FenceEdit,   // ELEMENTS
  BushEdit,
  TreeEdit,
  WaterEdit,
  ExitEdit,
  DogEdit,     // CHARACTERS
  SheepEdit,
  UndoEdit,    // EDITING
  EraserEdit,
  PanEdit,
  LoadEdit,
  SaveEdit,
  PlayEdit,
  LeaveEdit,
  NumEditTypes
};

enum XmlSaveError
{
  NoError,
  NoExitError,
  NoSheepError,
  NoDogError,
  DirCreateError,
  OpenFileError,
  WriteFileError
};

enum GameObjType
{
  ElmtObj,
  DogObj,
  SheepObj
};

enum LevelSource
{
  BuiltInSource,
  UserSource,
  InvalidSource
};

#endif /* LEVEL_TYPES_HPP_ */
