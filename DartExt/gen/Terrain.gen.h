
#pragma once

#include "../dartHelpers.h"

#include <SceneObjects/Terrain.h>
#include "SceneObject.gen.h"

class AtumExt_Terrain : public AtumExt_SceneObject
{
public:
  Terrain object;

public:
  virtual void* dart_ext_get_object() override
  {
    return &object;
  }

  /**
  * \returns null
  */
  Dart_Handle ShRender(float dt);
  /**
  * \returns null
  */
  Dart_Handle LoadHMap(const char * hgt_name);
  /**
  * \returns null
  */
  Dart_Handle Play();
  /**
  * \returns null
  */
  Dart_Handle Init();
  /**
  * \returns float
  */
  Dart_Handle GetHeight(int i, int j);
  /**
  * \returns Vector
  */
  Dart_Handle GetVecHeight(int i, int j);
  /**
  * \returns null
  */
  Dart_Handle Stop();
  Dart_Handle get_field_hwidth();
  Dart_Handle get_field_sz();
  Dart_Handle get_field_tex_name();
  Dart_Handle get_field_color();
  Dart_Handle get_field_hheight();
  Dart_Handle get_field_scaleh();
  Dart_Handle get_field_scalev();
  Dart_Handle get_field_hgt_name();
};
