
part of atum;

class Terrain extends SceneObject {
  ShRender(double dt) native "AtumExt_Terrain::ShRender";
  LoadHMap(String hgt_name) native "AtumExt_Terrain::LoadHMap";
  @override
  Play() native "AtumExt_Terrain::Play";
  @override
  Init() native "AtumExt_Terrain::Init";
  double _GetHeight() native "AtumExt_Terrain::GetHeight";
  Vector _GetVecHeight() native "AtumExt_Terrain::GetVecHeight";
  @override
  Stop() native "AtumExt_Terrain::Stop";
  int _get_field_hwidth() native "AtumExt_Terrain::get_field_hwidth";
  int _get_field_sz() native "AtumExt_Terrain::get_field_sz";
  std::string _get_field_tex_name() native "AtumExt_Terrain::get_field_tex_name";
  Float32List _get_field_color() native "AtumExt_Terrain::get_field_color";
  int _get_field_hheight() native "AtumExt_Terrain::get_field_hheight";
  double _get_field_scaleh() native "AtumExt_Terrain::get_field_scaleh";
  double _get_field_scalev() native "AtumExt_Terrain::get_field_scalev";
  std::string _get_field_hgt_name() native "AtumExt_Terrain::get_field_hgt_name";
  double get Height => _GetHeight();
  Vector get VecHeight => _GetVecHeight();
  int get hwidth => _get_field_hwidth();
  int get sz => _get_field_sz();
  std::string get tex_name => _get_field_tex_name();
  Vector4 get color => new Vector4.fromFloat32List(_get_field_color());
  int get hheight => _get_field_hheight();
  double get scaleh => _get_field_scaleh();
  double get scalev => _get_field_scalev();
  std::string get hgt_name => _get_field_hgt_name();

  _ctor() native "AtumExt_Terrain::AtumExt_Terrain";

  Terrain() {
    _ctor();
  }
}

