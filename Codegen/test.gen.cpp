
#include "../dartHelpers.h"

#include <Services/Scene/SceneObject.h>
#include <SceneObjects/PhysBox.h>

//#ifdef GetClassName
//#undef GetClassName
//#endif


class AtumExt_SceneObject : public RefCount
{
public:
  SceneObject *objectAsSceneObject = nullptr;

public:
  virtual SceneObject* dart_ext_get_object() = 0;

  void dart_ext_init()
  {
    objectAsSceneObject = (SceneObject*)dart_ext_get_object();
  }

  Dart_Handle GetName()
  {
    return fromValue(objectAsSceneObject->GetName());
  }

  Dart_Handle SetName(const char * name)
  {
    objectAsSceneObject->SetName(name);
    return Dart_Null();
  }

  Dart_Handle GetTrans()
  {
    return fromValue(objectAsSceneObject->Trans());
  }

  Dart_Handle SetTrans(const Matrix & value)
  {
    objectAsSceneObject->Trans() = value;
    return Dart_Null();
  }

  Dart_Handle Init()
  {
    objectAsSceneObject->Init();
    return Dart_Null();
  }

  Dart_Handle Play()
  {
    objectAsSceneObject->Play();
    return Dart_Null();
  }

  Dart_Handle Stop()
  {
    objectAsSceneObject->Stop();
    return Dart_Null();
  }

  Dart_Handle Playing()
  {
    return fromValue(objectAsSceneObject->Playing());
  }

  Dart_Handle Release()
  {
    objectAsSceneObject->Release();
    return Dart_Null();
  }

  Dart_Handle get_field_test()
  {
    return fromValue(object.test);
  }

};

NATIVE_METHOD_LIST_NO_CTOR(AtumExt_SceneObject,
  NATIVE_METHOD(AtumExt_SceneObject, GetName),
  NATIVE_METHOD(AtumExt_SceneObject, SetName, const char *),
  NATIVE_METHOD(AtumExt_SceneObject, GetTrans),
  NATIVE_METHOD(AtumExt_SceneObject, SetTrans, Matrix),
  NATIVE_METHOD(AtumExt_SceneObject, Init),
  NATIVE_METHOD(AtumExt_SceneObject, Play),
  NATIVE_METHOD(AtumExt_SceneObject, Stop),
  NATIVE_METHOD(AtumExt_SceneObject, Playing),
  NATIVE_METHOD(AtumExt_SceneObject, Release));

class AtumExt_PhysBox : public AtumExt_SceneObject
{
public:
  PhysBox object;

public:
  virtual SceneObject* dart_ext_get_object() override
  {
    return &object;
  }

  Dart_Handle Play()
  {
    object.Play();
    return Dart_Null();
  }

  Dart_Handle Draw(float dt)
  {
    object.Draw(dt);
    return Dart_Null();
  }

  Dart_Handle Init()
  {
    object.Init();
    return Dart_Null();
  }

  Dart_Handle Stop()
  {
    object.Stop();
    return Dart_Null();
  }

  Dart_Handle get_field_sizeX()
  {
    return fromValue(object.sizeX);
  }

  Dart_Handle get_field_sizeY()
  {
    return fromValue(object.sizeY);
  }

  Dart_Handle get_field_sizeZ()
  {
    return fromValue(object.sizeZ);
  }

  Dart_Handle get_field_isStatic()
  {
    return fromValue(object.isStatic);
  }

  Dart_Handle get_field_color()
  {
    return fromValue(object.color);
  }

};

NATIVE_METHOD_LIST(AtumExt_PhysBox,
  NATIVE_METHOD(AtumExt_PhysBox, Play),
  NATIVE_METHOD(AtumExt_PhysBox, Draw, float),
  NATIVE_METHOD(AtumExt_PhysBox, Init),
  NATIVE_METHOD(AtumExt_PhysBox, Stop));

