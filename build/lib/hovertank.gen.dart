
library atum;

import 'dart:io';
import 'dart:async';
import 'dart:isolate';

import 'dart:typed_data';
import 'package:vector_math/vector_math.dart';

import 'dart-ext:AtumExt';

abstract class _Native extends NativeClass1 {
}

class HoverTank extends SceneObject {
  AddSplash(Float32List pos, double radius, double force) native "AtumExt_HoverTank::AddSplash";
  @override
  Play() native "AtumExt_HoverTank::Play";
  @override
  Init() native "AtumExt_HoverTank::Init";
  @override
  Stop() native "AtumExt_HoverTank::Stop";
  Update(double dt) native "AtumExt_HoverTank::Update";
  AddHover(Float32List mat, Vector offset) native "AtumExt_HoverTank::AddHover";
  int _get_field_alias_strafe() native "AtumExt_HoverTank::get_field_alias_strafe";
  bool _get_field_showDebug() native "AtumExt_HoverTank::get_field_showDebug";
  std::vector<Projectile> _get_field_projectiles() native "AtumExt_HoverTank::get_field_projectiles";
  std::vector<Box> _get_field_boxes() native "AtumExt_HoverTank::get_field_boxes";
  Vector2 _get_field_angles() native "AtumExt_HoverTank::get_field_angles";
  Matrix _get_field_view() native "AtumExt_HoverTank::get_field_view";
  double _get_field_strafe_speed() native "AtumExt_HoverTank::get_field_strafe_speed";
  Matrix _get_field_proj() native "AtumExt_HoverTank::get_field_proj";
  double _get_field_move_speed() native "AtumExt_HoverTank::get_field_move_speed";
  int _get_field_alias_forward() native "AtumExt_HoverTank::get_field_alias_forward";
  int _get_field_alias_fast() native "AtumExt_HoverTank::get_field_alias_fast";
  int _get_field_alias_rotate_active() native "AtumExt_HoverTank::get_field_alias_rotate_active";
  int _get_field_alias_rotate_x() native "AtumExt_HoverTank::get_field_alias_rotate_x";
  int _get_field_alias_rotate_y() native "AtumExt_HoverTank::get_field_alias_rotate_y";
  double _get_field_physStep() native "AtumExt_HoverTank::get_field_physStep";
  double _get_field_accum_dt() native "AtumExt_HoverTank::get_field_accum_dt";
  Model _get_field_hover_model() native "AtumExt_HoverTank::get_field_hover_model";
  Model _get_field_tower_model() native "AtumExt_HoverTank::get_field_tower_model";
  Model _get_field_gun_model() native "AtumExt_HoverTank::get_field_gun_model";
  int get alias_strafe => _get_field_alias_strafe();
  bool get showDebug => _get_field_showDebug();
  std::vector<Projectile> get projectiles => _get_field_projectiles();
  std::vector<Box> get boxes => _get_field_boxes();
  Vector2 get angles => _get_field_angles();
  Matrix get view => _get_field_view();
  double get strafe_speed => _get_field_strafe_speed();
  Matrix get proj => _get_field_proj();
  double get move_speed => _get_field_move_speed();
  int get alias_forward => _get_field_alias_forward();
  int get alias_fast => _get_field_alias_fast();
  int get alias_rotate_active => _get_field_alias_rotate_active();
  int get alias_rotate_x => _get_field_alias_rotate_x();
  int get alias_rotate_y => _get_field_alias_rotate_y();
  double get physStep => _get_field_physStep();
  double get accum_dt => _get_field_accum_dt();
  Model get hover_model => _get_field_hover_model();
  Model get tower_model => _get_field_tower_model();
  Model get gun_model => _get_field_gun_model();

  _ctor() native "AtumExt_HoverTank::AtumExt_HoverTank";

  HoverTank() {
    _ctor();
  }
}

