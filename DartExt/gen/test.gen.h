
#pragma once

#include "../dartHelpers.h"


class AtumExt_PhysBox : public AtumExt_SceneObject
{
public:
  PhysBox object;

public:
  virtual SceneObject* dart_ext_get_object() override
  {
    return &object;
  }

  /**
  * \returns null
  */
  Dart_Handle Play();
  /**
  * \returns null
  */
  Dart_Handle Draw(float dt);
  /**
  * \returns null
  */
  Dart_Handle Init();
  /**
  * \returns null
  */
  Dart_Handle Stop();
  Dart_Handle get_field_sizeX();
  Dart_Handle get_field_sizeY();
  Dart_Handle get_field_sizeZ();
  Dart_Handle get_field_isStatic();
  Dart_Handle get_field_color();
};
