
#pragma once

#include "../dartHelpers.h"

#include <Services/Scene/Scene.h>
#include "SceneObject.gen.h"

class AtumExt_Scene : public AtumExtClass
{
public:
  Scene object;

public:
  virtual void* dart_ext_get_object() override
  {
    return &object;
  }

  /**
  * \returns null
  */
  Dart_Handle Load(const char * name);
  /**
  * \returns null
  */
  Dart_Handle DeleteObject(SceneObject * obj);
  /**
  * \returns null
  */
  Dart_Handle DelFromGroup(SceneObject * obj, const char * name);
  /**
  * \returns null
  */
  Dart_Handle Play();
  /**
  * \returns SceneObject *
  */
  Dart_Handle AddObject(const char * name);
  /**
  * \returns null
  */
  Dart_Handle Init();
  /**
  * \returns SceneObject *
  */
  Dart_Handle Find(const char * name);
  /**
  * \returns int
  */
  Dart_Handle GetObjectsCount();
  /**
  * \returns SceneObject *
  */
  Dart_Handle GetObj(int index);
  /**
  * \returns null
  */
  Dart_Handle DeleteAllObjects();
  /**
  * \returns null
  */
  Dart_Handle Save(const char * name);
  /**
  * \returns null
  */
  Dart_Handle Execute(float dt);
  /**
  * \returns null
  */
  Dart_Handle Stop();
  /**
  * \returns null
  */
  Dart_Handle DelFromAllGroups(SceneObject * obj);
  /**
  * \returns bool
  */
  Dart_Handle Playing();
  /**
  * \returns null
  */
  Dart_Handle AddToGroup(SceneObject * obj, const char * name);
};
