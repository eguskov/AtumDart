// Copyright (c) 2012, the Dart project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

library atum_extension;

import 'dart:async';
import 'dart:io';

import 'package:atum/atum_extension.dart';
import 'package:vector_math/vector_math.dart';

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
AtumTank tank;

Timer timer;

int alias_forward = -1;
int alias_strafe = -1;
int alias_fast = -1;
int alias_rotate_active = -1;
int alias_rotate_x = -1;
int alias_rotate_y = -1;

double move_speed = 0.0;
double strafe_speed = 0.0;

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
}

main() async {
  core = new AtumCore();
  core.init();

  scene = core.addScene();
  print(scene);

  var iso = await core.loadSceneIsolate('lib/atum_scene_worker.dart');

  scene.load("Media/Scene.scn");
  print(scene.getObjectsCount());
  print(scene.getObject(0));

  for (var i = 0, count = scene.getObjectsCount(); i < count; ++i) {
    var object = scene.getObject(i);
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

  // await new Timer(const Duration(seconds: 5), () => {});

  timer = new Timer.periodic(const Duration(milliseconds: 10), (_) => update());
}
