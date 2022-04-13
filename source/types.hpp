#ifndef TYPES_HPP_
#define TYPES_HPP_

enum GameMode
{
  NoGameMode,
  MainMenuMode,
  BuiltInChooserMode,
  UserChooserMode,
  EditChooserMode,
  PlayMode,
  PausedMode,
  LevelEditMode,
  NumGameModes
};

enum GraphicQuality
{
  LowQuality,
  MedQuality,
  HighQuality,
  VeryHighQuality
};

enum ImageType
{
  UnknownImgType,
  PngImgType,
  PngImgListType,
  SvgImgType,
  CustomImgType
};

#endif /* TYPES_HPP_ */
