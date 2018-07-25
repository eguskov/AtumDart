#include "Scene.gen.h"

/**
* \returns null
*/
Dart_Handle AtumExt_Scene::Load(const char * name)
{
  object.Load(name);
  return Dart_Null();
}

/**
* \returns null
*/
Dart_Handle AtumExt_Scene::DeleteObject(SceneObject * obj)
{
  object.DeleteObject(obj);
  return Dart_Null();
}

/**
* \returns null
*/
Dart_Handle AtumExt_Scene::DelFromGroup(SceneObject * obj, const char * name)
{
  object.DelFromGroup(obj, name);
  return Dart_Null();
}

/**
* \returns null
*/
Dart_Handle AtumExt_Scene::Play()
{
  object.Play();
  return Dart_Null();
}

/**
* \returns SceneObject *
*/
Dart_Handle AtumExt_Scene::AddObject(const char * name)
{
  return nativeToDart(object.AddObject(name));
}

/**
* \returns null
*/
Dart_Handle AtumExt_Scene::Init()
{
  object.Init();
  return Dart_Null();
}

/**
* \returns SceneObject *
*/
Dart_Handle AtumExt_Scene::Find(const char * name)
{
  return nativeToDart(object.Find(name));
}

/**
* \returns int
*/
Dart_Handle AtumExt_Scene::GetObjectsCount()
{
  return nativeToDart(object.GetObjectsCount());
}

/**
* \returns SceneObject *
*/
Dart_Handle AtumExt_Scene::GetObj(int index)
{
  return nativeToDart(object.GetObj(index));
}

/**
* \returns null
*/
Dart_Handle AtumExt_Scene::DeleteAllObjects()
{
  object.DeleteAllObjects();
  return Dart_Null();
}

/**
* \returns null
*/
Dart_Handle AtumExt_Scene::Save(const char * name)
{
  object.Save(name);
  return Dart_Null();
}

/**
* \returns null
*/
Dart_Handle AtumExt_Scene::Execute(float dt)
{
  object.Execute(dt);
  return Dart_Null();
}

/**
* \returns null
*/
Dart_Handle AtumExt_Scene::Stop()
{
  object.Stop();
  return Dart_Null();
}

/**
* \returns null
*/
Dart_Handle AtumExt_Scene::DelFromAllGroups(SceneObject * obj)
{
  object.DelFromAllGroups(obj);
  return Dart_Null();
}

/**
* \returns bool
*/
Dart_Handle AtumExt_Scene::Playing()
{
  return nativeToDart(object.Playing());
}

/**
* \returns null
*/
Dart_Handle AtumExt_Scene::AddToGroup(SceneObject * obj, const char * name)
{
  object.AddToGroup(obj, name);
  return Dart_Null();
}

NATIVE_METHOD_LIST(AtumExt_Scene,
  NATIVE_METHOD(AtumExt_Scene, Load, const char *),
  NATIVE_METHOD(AtumExt_Scene, DeleteObject, SceneObject *),
  NATIVE_METHOD(AtumExt_Scene, DelFromGroup, SceneObject *, const char *),
  NATIVE_METHOD(AtumExt_Scene, Play),
  NATIVE_METHOD(AtumExt_Scene, AddObject, const char *),
  NATIVE_METHOD(AtumExt_Scene, Init),
  NATIVE_METHOD(AtumExt_Scene, Find, const char *),
  NATIVE_METHOD(AtumExt_Scene, GetObjectsCount),
  NATIVE_METHOD(AtumExt_Scene, GetObj, int),
  NATIVE_METHOD(AtumExt_Scene, DeleteAllObjects),
  NATIVE_METHOD(AtumExt_Scene, Save, const char *),
  NATIVE_METHOD(AtumExt_Scene, Execute, float),
  NATIVE_METHOD(AtumExt_Scene, Stop),
  NATIVE_METHOD(AtumExt_Scene, DelFromAllGroups, SceneObject *),
  NATIVE_METHOD(AtumExt_Scene, Playing),
  NATIVE_METHOD(AtumExt_Scene, AddToGroup, SceneObject *, const char *));

