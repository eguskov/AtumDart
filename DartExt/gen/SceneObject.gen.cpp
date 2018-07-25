#include "SceneObject.gen.h"

template<>
inline Dart_Handle nativeToDart(const char & value)
{
  return Dart_Null();
}

/**
* \returns const char *
*/
Dart_Handle AtumExt_SceneObject::GetName()
{
  return nativeToDart(objectAsSceneObject->GetName());
}

/**
* \returns null
*/
Dart_Handle AtumExt_SceneObject::SetName(const char * name)
{
  objectAsSceneObject->SetName(name);
  return Dart_Null();
}

/**
* \returns const char *
*/
Dart_Handle AtumExt_SceneObject::GetClassNameA()
{
  return nativeToDart(objectAsSceneObject->GetClassNameA());
}

/**
* \returns Matrix &
*/
Dart_Handle AtumExt_SceneObject::GetTrans()
{
  return nativeToDart(objectAsSceneObject->Trans());
}

/**
* \returns null
*/
Dart_Handle AtumExt_SceneObject::SetTrans(const Matrix & value)
{
  objectAsSceneObject->Trans() = value;
  return Dart_Null();
}

/**
* \returns null
*/
Dart_Handle AtumExt_SceneObject::Init()
{
  objectAsSceneObject->Init();
  return Dart_Null();
}

/**
* \returns null
*/
Dart_Handle AtumExt_SceneObject::Play()
{
  objectAsSceneObject->Play();
  return Dart_Null();
}

/**
* \returns null
*/
Dart_Handle AtumExt_SceneObject::Stop()
{
  objectAsSceneObject->Stop();
  return Dart_Null();
}

/**
* \returns bool
*/
Dart_Handle AtumExt_SceneObject::Playing()
{
  return nativeToDart(objectAsSceneObject->Playing());
}

/**
* \returns null
*/
Dart_Handle AtumExt_SceneObject::Release()
{
  objectAsSceneObject->Release();
  return Dart_Null();
}

Dart_Handle AtumExt_SceneObject::get_field_test()
{
  return nativeToDart(objectAsSceneObject->test);
}

NATIVE_METHOD_LIST_NO_CTOR(AtumExt_SceneObject,
  NATIVE_METHOD(AtumExt_SceneObject, GetName),
  NATIVE_METHOD(AtumExt_SceneObject, SetName, const char *),
  NATIVE_METHOD(AtumExt_SceneObject, GetClassNameA),
  NATIVE_METHOD(AtumExt_SceneObject, GetTrans),
  NATIVE_METHOD(AtumExt_SceneObject, SetTrans, Matrix),
  NATIVE_METHOD(AtumExt_SceneObject, Init),
  NATIVE_METHOD(AtumExt_SceneObject, Play),
  NATIVE_METHOD(AtumExt_SceneObject, Stop),
  NATIVE_METHOD(AtumExt_SceneObject, Playing),
  NATIVE_METHOD(AtumExt_SceneObject, Release),
  NATIVE_METHOD(AtumExt_SceneObject, get_field_test));

