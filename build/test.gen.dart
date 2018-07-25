
library atum;

import 'dart:io';
import 'dart:async';
import 'dart:isolate';

import 'dart:typed_data';
import 'package:vector_math/vector_math.dart';

import 'dart-ext:AtumExt';

abstract class _Native extends NativeClass1 {
}

class SceneObject extends _Native {
  String _GetName() native "AtumExt_SceneObject::GetName";
  _SetName(String) native "AtumExt_SceneObject::SetName";
  Float32List _GetTrans() native "AtumExt_SceneObject::GetTrans";
  _SetTrans(Float32List) native "AtumExt_SceneObject::SetTrans";
  _Init() native "AtumExt_SceneObject::Init";
  _Play() native "AtumExt_SceneObject::Play";
  _Stop() native "AtumExt_SceneObject::Stop";
  bool _Playing() native "AtumExt_SceneObject::Playing";
  _Release() native "AtumExt_SceneObject::Release";

  String get Name => _GetName();
  set Name(String v) => _SetName(v);
  Matrix4 get Trans => new Matrix4.fromFloat32List(_GetTrans());
  set Trans(Matrix4 v) => _SetTrans(v.storage);
}


class PhysBox extends SceneObject {
  _Play() native "AtumExt_PhysBox::Play";
  _Draw(double) native "AtumExt_PhysBox::Draw";
  _Init() native "AtumExt_PhysBox::Init";
  _Stop() native "AtumExt_PhysBox::Stop";

  _ctor() native "AtumExt_PhysBox::AtumExt_PhysBox";

  PhysBox() {
    _ctor();
  }
}

