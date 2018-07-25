#include "PhysBox.gen.h"

/**
* \returns null
*/
Dart_Handle AtumExt_PhysBox::Play()
{
  object.Play();
  return Dart_Null();
}

/**
* \returns null
*/
Dart_Handle AtumExt_PhysBox::Draw(float dt)
{
  object.Draw(dt);
  return Dart_Null();
}

/**
* \returns null
*/
Dart_Handle AtumExt_PhysBox::Init()
{
  object.Init();
  return Dart_Null();
}

/**
* \returns null
*/
Dart_Handle AtumExt_PhysBox::Stop()
{
  object.Stop();
  return Dart_Null();
}

Dart_Handle AtumExt_PhysBox::get_field_sizeX()
{
  return nativeToDart(object.sizeX);
}

Dart_Handle AtumExt_PhysBox::get_field_sizeY()
{
  return nativeToDart(object.sizeY);
}

Dart_Handle AtumExt_PhysBox::get_field_sizeZ()
{
  return nativeToDart(object.sizeZ);
}

Dart_Handle AtumExt_PhysBox::get_field_isStatic()
{
  return nativeToDart(object.isStatic);
}

Dart_Handle AtumExt_PhysBox::get_field_color()
{
  return nativeToDart(object.color);
}

NATIVE_METHOD_LIST(AtumExt_PhysBox,
  NATIVE_METHOD(AtumExt_PhysBox, Play),
  NATIVE_METHOD(AtumExt_PhysBox, Draw, float),
  NATIVE_METHOD(AtumExt_PhysBox, Init),
  NATIVE_METHOD(AtumExt_PhysBox, Stop),
  NATIVE_METHOD(AtumExt_PhysBox, get_field_sizeX),
  NATIVE_METHOD(AtumExt_PhysBox, get_field_sizeY),
  NATIVE_METHOD(AtumExt_PhysBox, get_field_sizeZ),
  NATIVE_METHOD(AtumExt_PhysBox, get_field_isStatic),
  NATIVE_METHOD(AtumExt_PhysBox, get_field_color));

