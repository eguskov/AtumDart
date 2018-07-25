// Copyright (c) 2012, the Dart project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

library atum_extension;

import 'dart:async';
import 'dart:io';
import 'dart:math';

import 'package:atum/atum_extension.dart';
import 'package:atum/protocol.dart' as proto;

import 'package:vector_math/vector_math.dart';



class Projectile
	{
		static double maxTimeLife = 4.0;
		static double speed = 50.0;
		static double splashTime = 0.35;
		static double splashMaxRadius = 5.0;

		Vector3 pos;
		Vector3 dir;
		int    stage;
		double  lifetime;
		int    state;
	}

void check(bool condition, String message) {
  if (!condition) {
    throw new StateError(message);
  }
}

void checkSystemRand() {
  systemSrand(17);
  var x1 = systemRand();
  var x2 = systemRand();
  var x3 = systemRand();
  check(x1 != x2, "x1 != x2");
  check(x1 != x3, "x1 != x3");
  check(x2 != x3, "x2 != x3");
  systemSrand(17);
  check(x1 == systemRand(), "x1 == systemRand()");
  check(x2 == systemRand(), "x2 == systemRand()");
  check(x3 == systemRand(), "x3 == systemRand()");
  systemSrand(18);
  check(x1 != systemRand(), "x1 != systemRand()");
  check(x2 != systemRand(), "x2 != systemRand()");
  check(x3 != systemRand(), "x3 != systemRand()");
}

void checkNoScopeSystemRand() {
  systemSrand(17);
  var x1 = noScopeSystemRand();
  var x2 = noScopeSystemRand();
  var x3 = noScopeSystemRand();
  check(x1 != x2, "x1 != x2");
  check(x1 != x3, "x1 != x3");
  check(x2 != x3, "x2 != x3");
  systemSrand(17);
  check(x1 == noScopeSystemRand(), "x1 == noScopeSystemRand()");
  check(x2 == noScopeSystemRand(), "x2 == noScopeSystemRand()");
  check(x3 == noScopeSystemRand(), "x3 == noScopeSystemRand()");
  systemSrand(18);
  check(x1 != noScopeSystemRand(), "x1 != noScopeSystemRand()");
  check(x2 != noScopeSystemRand(), "x2 != noScopeSystemRand()");
  check(x3 != noScopeSystemRand(), "x3 != noScopeSystemRand()");
}

AtumCore core;
AtumScene scene;
AtumPhysScene pscene;
AtumTank tank;

List<Projectile> projectiles = [];

Timer timer;

int alias_forward = -1;
int alias_strafe = -1;
int alias_fast = -1;
int alias_rotate_active = -1;
int alias_rotate_x = -1;
int alias_rotate_y = -1;

double move_speed = 0.0;
double strafe_speed = 0.0;
double shoot_cooldown = 0.0;

update() {
  double dt = core.update();
  if (dt < 0.0) {
    timer.cancel();
    exit(0);
    return;
  }

  if (core.controlsIsDebugKeyActive("KEY_W")) {
    move_speed += 20.0 * dt;
    if (move_speed > 30.0) {
      move_speed = 30.0;
    }
  } else if (core.controlsIsDebugKeyActive("KEY_S")) {
    move_speed -= 15 * dt;
    if (move_speed < -20.0) {
      move_speed = -20.0;
    }
  } else {
    if (move_speed > 0.1) {
      move_speed -= 30 * dt;

      if (move_speed < 0) {
        move_speed = 0.0;
      }
    } else if (move_speed < 0.1) {
      move_speed += 30.0 * dt;

      if (move_speed > 0) {
        move_speed = 0.0;
      }
    }
  }

  if (core.controlsIsDebugKeyActive("KEY_D")) {
    strafe_speed += 15.0 * dt;

    if (strafe_speed > 3.0) {
      strafe_speed = 3.0;
    }
  } else if (core.controlsIsDebugKeyActive("KEY_A")) {
    strafe_speed -= 15.0 * dt;

    if (strafe_speed < -3.0) {
      strafe_speed = -3.0;
    }
  } else {
    if (strafe_speed > 0.1) {
      strafe_speed -= 15.0 * dt;

      if (strafe_speed < 0.0) {
        strafe_speed = 0.0;
      }
    } else if (strafe_speed < 0.1) {
      strafe_speed += 15.0 * dt;

      if (strafe_speed > 0) {
        strafe_speed = 0.0;
      }
    }
  }

  Matrix4 tm = new Matrix4.identity();
  tm.rotateY(tank.angles.z);
  tm.translate(controller->GetPosition());

  Matrix4 view = makeViewMatrix(tm.getTranslation() + new Vector3(0.0, 4.5, 0.0) - new Vector3(cos(tank.angles.x), sin(tank.angles.y), sin(tank.angles.x)) * 55.0, tm.getTranslation() + new Vector3(0.0, 4.5, 0.0), new Vector3(0.0, 1.0, 0.0));
  Matrix4 proj = makePerspectiveMatrix(45.0 * (180.0 / PI), core.renderGetHeight() / core.renderGetWidth(), 1.0, 1000.0);

  double tower_angel = tank.angles.z;

	Vector3 target_pt = new Vector3.zero();

	RaycastDesc rcdesc = new RaycastDesc();

	{
		Vector2 screepos = new Vector2(core.controlsGetAliasValue(alias_rotate_x, false) / core.renderGetWidth(),
									core.controlsGetAliasValue(alias_rotate_y, false) / core.renderGetHeight());

		Vector3 v;
		v.x = (2.0 * screepos.x - 1.0) / proj.entry(0, 0);
		v.y = -(2.0 * screepos.y - 1.0) / proj.entry(1, 1);
		v.z = 1.0;

    Matrix4 invView = new Matrix4.inverted(view);

		Vector3 camPos = invView.getTranslation();

		Vector3 dir;
		dir.x = v.x * invView.entry(0, 0) + v.y * invView.entry(1, 0) + v.z * invView.entry(2, 0);
		dir.y = v.x * invView.entry(0, 1) + v.y * invView.entry(1, 1) + v.z * invView.entry(2, 1);
		dir.z = v.x * invView.entry(0, 2) + v.y * invView.entry(1, 2) + v.z * invView.entry(2, 2);
		
		// Vector3 screen = camPos + dir;
		dir.normalize();

		rcdesc.origin = camPos;
		rcdesc.dir = dir;
		rcdesc.length = 500.0;

		if (pscene->RayCast(rcdesc))
		{
			target_pt = rcdesc.hitPos;
			render.DebugSphere(target_pt, COLOR_RED, 0.5);

			dir = target_pt - tm.getTranslation();
			dir.normalize();

			tower_angel = acos(dir.dot(new Vector3(1.0,0.0,0.0)));

			if (dir.dot(new Vector3(0.0, 0.0, 1.0)) > 0.0)
			{
				tower_angel = PI * 2 - tower_angel;
			}
		}
	}

  Vector3 dir = tm.getColumn(0).xyz;
  dir.y = 0.0;
  dir.normalize();

  Vector3 angles = tank.angles;
  angles.z += strafe_speed * dt;
  tank.angles = angles;

  dir *= move_speed;
  dir.y = -9.8;
  dir *= dt;
  tank.move(dir);

  		double under = 1.0;

		rcdesc.dir = new Vector3(0.0, -1.0, 0.0);
		rcdesc.length = under + 2.0;

		Vector3 org = tm.getTranslation();
		org.y += under;

		Vector3 p1 = org + tm.getColumn(0).xyz * 1.753;
		rcdesc.origin = p1;

		if (pscene.rayCast(rcdesc))
		{
			p1 = rcdesc.hitPos;
		}
		else
		{
			p1.y -= under;
		}

		Vector3 p2 = org - tm.getColumn(0).xyz * 1.75 - tm.getColumn(2).xyz;
		rcdesc.origin = p2;

		if (pscene.rayCast(rcdesc))
		{
			p2 = rcdesc.hitPos;
		}
		else
		{
			p2.y -= under;
		}

		Vector p3 = org - tm.getColumn(0).xyz * 1.75 + tm.getColumn(2).xyz;
		rcdesc.origin = p3;

		if (pscene.rayCast(rcdesc))
		{
			p3 = rcdesc.hitPos;
		}
		else
		{
			p3.y -= under;
		}

  mat.Vz() = p3 - p2;
		mat.Vz().Normalize();

		mat.Vx() = p1 - (p3 + p2)*0.5;
		mat.Vx().Normalize();

		mat.Vy() = mat.Vz().Cross(mat.Vx());

    Matrix mdl = mat;
		hover_drawer->SetTransform(mdl);

		mdl = Matrix().RotateY(tower_angel - angles.z) * Matrix().Move(hover_model.locator) * mdl;
		tower_drawer->SetTransform(mdl);

		mdl = Matrix().Move(tower_model.locator) * mdl;
		gun_drawer->SetTransform(mdl);
  
  if (shoot_cooldown > 0.0)
		{
			shoot_cooldown -= dt;
		}

  if (core.controlsIsDebugKeyPressed("MS_BTN0") && shoot_cooldown <= 0.0)
		{
			mdl = Matrix().Move(gun_model.locator) * mdl;

			Projectile proj = new Projectile();
			projectiles.add(proj);

			proj.pos = mdl.Pos();
			proj.dir = target_pt - proj.pos;
			proj.dir.Normalize();
			proj.lifetime = Projectile.maxTimeLife;
			proj.state = 0;

			shoot_cooldown = 1.5;
		}
}

main() async {
  core = new AtumCore();
  core.init();

  scene = core.addScene();
  print(scene);

  IsolateConnection conn = await core.loadSceneIsolate('lib/atum_scene_worker.dart');
  print(conn);

  conn.send(const proto.SceneControlPlay());

  scene.load("Media/Scene.scn");
  print(scene.getObjectsCount());
  //print(scene.getObject(0));

  for (var i = scene.getObjectsCount() - 1; i >= 0; --i) {
    var object = scene.getObject(i);
    print(object);
    print(
        'object[$i]: name: ${object.getName()}; class: ${object.getClassName()}');
  }

  var m = scene.getObject(0).trans;
  print(m);
  scene.getObject(0).trans = m;

  alias_forward = core.controlsGetAlias("MOVE_FORWARD");
  alias_strafe = core.controlsGetAlias("MOVE_STRAFE");
  alias_fast = core.controlsGetAlias("MOVE_FAST");
  alias_rotate_active = core.controlsGetAlias("ROTATE_ACTIVE");
  alias_rotate_x = core.controlsGetAlias("ROTATE_X");
  alias_rotate_y = core.controlsGetAlias("ROTATE_Y");

  scene.play();

  tank = scene.getObject(1).cast(AtumTank) as AtumTank;

  // scene.addObject("Terrain");

  //AtumScene scene = new AtumScene();
  //scene.addObject("Terrain");

  //checkSystemRand();
  //checkNoScopeSystemRand();

  //RandomArray r = new RandomArray();
  //var arr = await r.randomArray(17, 100);
  //print(arr); 

  timer = new Timer.periodic(const Duration(milliseconds: 10), (_) => update());
}
