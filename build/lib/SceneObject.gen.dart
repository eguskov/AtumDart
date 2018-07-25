
part of atum;

class SceneObject extends _Native {
  String _GetName() native "AtumExt_SceneObject::GetName";
  _SetName(String) native "AtumExt_SceneObject::SetName";
  String _GetClassNameA() native "AtumExt_SceneObject::GetClassNameA";
  Float32List _GetTrans() native "AtumExt_SceneObject::GetTrans";
  _SetTrans(Float32List) native "AtumExt_SceneObject::SetTrans";
  Init();
  Play();
  Stop();
  bool Playing() native "AtumExt_SceneObject::Playing";
  Release();
  bool _get_field_test() native "AtumExt_SceneObject::get_field_test";
  String get Name => _GetName();
  set Name(String v) => _SetName(v);
  String get ClassNameA => _GetClassNameA();
  Matrix4 get Trans => new Matrix4.fromFloat32List(_GetTrans());
  set Trans(Matrix4 v) => _SetTrans(v.storage);
  bool get test => _get_field_test();

  _ctor() native "AtumExt_SceneObject::AtumExt_SceneObject";

  SceneObject() {
    _ctor();
  }
}

