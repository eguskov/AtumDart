
#pragma once

#include "../dartHelpers.h"

#include <Services/Scene/SceneObject.h>

class AtumExt_SceneObject : public AtumExtClass
{
public:
  SceneObject *objectAsSceneObject = nullptr;

public:
  void dart_ext_init()
  {
    objectAsSceneObject = (SceneObject*)dart_ext_get_object();
  }

  /**
  * \returns const char *
  */
  Dart_Handle GetName();
  /**
  * \returns null
  */
  Dart_Handle SetName(const char * name);
  /**
  * \returns const char *
  */
  Dart_Handle GetClassNameA();
  /**
  * \returns Matrix &
  */
  Dart_Handle GetTrans();
  /**
  * \returns null
  */
  Dart_Handle SetTrans(const Matrix & value);
  /**
  * \returns null
  */
  Dart_Handle Init();
  /**
  * \returns null
  */
  Dart_Handle Play();
  /**
  * \returns null
  */
  Dart_Handle Stop();
  /**
  * \returns bool
  */
  Dart_Handle Playing();
  /**
  * \returns null
  */
  Dart_Handle Release();
  Dart_Handle get_field_test();
};
