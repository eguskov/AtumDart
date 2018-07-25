
part of atum;

class PhysBox extends SceneObject {
  @override
  Play() native "AtumExt_PhysBox::Play";
  Draw(double dt) native "AtumExt_PhysBox::Draw";
  @override
  Init() native "AtumExt_PhysBox::Init";
  @override
  Stop() native "AtumExt_PhysBox::Stop";
  double _get_field_sizeX() native "AtumExt_PhysBox::get_field_sizeX";
  double _get_field_sizeY() native "AtumExt_PhysBox::get_field_sizeY";
  double _get_field_sizeZ() native "AtumExt_PhysBox::get_field_sizeZ";
  bool _get_field_isStatic() native "AtumExt_PhysBox::get_field_isStatic";
  Float32List _get_field_color() native "AtumExt_PhysBox::get_field_color";
  double get sizeX => _get_field_sizeX();
  double get sizeY => _get_field_sizeY();
  double get sizeZ => _get_field_sizeZ();
  bool get isStatic => _get_field_isStatic();
  Vector4 get color => new Vector4.fromFloat32List(_get_field_color());

  _ctor() native "AtumExt_PhysBox::AtumExt_PhysBox";

  PhysBox() {
    _ctor();
  }
}

