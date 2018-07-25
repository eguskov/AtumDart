#include "Terrain.gen.h"

template<>
inline Dart_Handle nativeToDart(const std::string & value)
{
  return Dart_Null();
}

/**
* \returns null
*/
Dart_Handle AtumExt_Terrain::ShRender(float dt)
{
  object.ShRender(dt);
  return Dart_Null();
}

/**
* \returns null
*/
Dart_Handle AtumExt_Terrain::LoadHMap(const char * hgt_name)
{
  object.LoadHMap(hgt_name);
  return Dart_Null();
}

/**
* \returns null
*/
Dart_Handle AtumExt_Terrain::Play()
{
  object.Play();
  return Dart_Null();
}

/**
* \returns null
*/
Dart_Handle AtumExt_Terrain::Init()
{
  object.Init();
  return Dart_Null();
}

/**
* \returns float
*/
Dart_Handle AtumExt_Terrain::GetHeight(int i, int j)
{
  return nativeToDart(object.GetHeight(i, j));
}

/**
* \returns Vector
*/
Dart_Handle AtumExt_Terrain::GetVecHeight(int i, int j)
{
  return nativeToDart(object.GetVecHeight(i, j));
}

/**
* \returns null
*/
Dart_Handle AtumExt_Terrain::Stop()
{
  object.Stop();
  return Dart_Null();
}

Dart_Handle AtumExt_Terrain::get_field_hwidth()
{
  return nativeToDart(object.hwidth);
}

Dart_Handle AtumExt_Terrain::get_field_sz()
{
  return nativeToDart(object.sz);
}

Dart_Handle AtumExt_Terrain::get_field_tex_name()
{
  return nativeToDart(object.tex_name);
}

Dart_Handle AtumExt_Terrain::get_field_color()
{
  return nativeToDart(object.color);
}

Dart_Handle AtumExt_Terrain::get_field_hheight()
{
  return nativeToDart(object.hheight);
}

Dart_Handle AtumExt_Terrain::get_field_scaleh()
{
  return nativeToDart(object.scaleh);
}

Dart_Handle AtumExt_Terrain::get_field_scalev()
{
  return nativeToDart(object.scalev);
}

Dart_Handle AtumExt_Terrain::get_field_hgt_name()
{
  return nativeToDart(object.hgt_name);
}

NATIVE_METHOD_LIST(AtumExt_Terrain,
  NATIVE_METHOD(AtumExt_Terrain, ShRender, float),
  NATIVE_METHOD(AtumExt_Terrain, LoadHMap, const char *),
  NATIVE_METHOD(AtumExt_Terrain, Play),
  NATIVE_METHOD(AtumExt_Terrain, Init),
  NATIVE_METHOD(AtumExt_Terrain, GetHeight, int, int),
  NATIVE_METHOD(AtumExt_Terrain, GetVecHeight, int, int),
  NATIVE_METHOD(AtumExt_Terrain, Stop),
  NATIVE_METHOD(AtumExt_Terrain, get_field_hwidth),
  NATIVE_METHOD(AtumExt_Terrain, get_field_sz),
  NATIVE_METHOD(AtumExt_Terrain, get_field_tex_name),
  NATIVE_METHOD(AtumExt_Terrain, get_field_color),
  NATIVE_METHOD(AtumExt_Terrain, get_field_hheight),
  NATIVE_METHOD(AtumExt_Terrain, get_field_scaleh),
  NATIVE_METHOD(AtumExt_Terrain, get_field_scalev),
  NATIVE_METHOD(AtumExt_Terrain, get_field_hgt_name));

