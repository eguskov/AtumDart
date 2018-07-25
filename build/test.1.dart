// Copyright (c) 2012, the Dart project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

library atum_extension;

import 'dart:async';
import 'dart:io';
import 'dart:math';

import 'package:atum/atum_extension.dart';

main() {
  PhysBox box = new PhysBox();
  print(box);

  AtumCore core = new AtumCore();
  print(core);

  core.init();

  Scene scene = new Scene();
  print(scene);

  core.addScene(scene);
}
