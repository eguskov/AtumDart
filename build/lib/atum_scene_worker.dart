import 'dart:async';
import 'dart:isolate';
import 'dart:typed_data';
import 'dart:convert';

import 'package:atum/atum_extension.dart';
import 'package:atum/protocol.dart' as proto;
import 'package:vector_math/vector_math.dart';

import 'src/protocol.dart';

AtumCore core;
AtumScene scene;

SendPort mainPort;
ReceivePort commandPort = new ReceivePort();

Timer timer;

onCommandMessage(message) {
  print('Worker: ${message}');

  message = proto.deserialize(message);

  if (message is proto.SceneControlPlay) {
    print('Worker: scene.play()');
    scene.play();
  }
}

update() {}

main(args) {
  commandPort.listen(onCommandMessage);

  if (args[0] is SendPort) {
    mainPort = args[0];
    mainPort.send(commandPort.sendPort);
  }

  core = new AtumCore();

  scene = core.addScene()..load('Media/dynamic.scn');

  for (var i = scene.getObjectsCount() - 1; i >= 0; --i) {
    var object = scene.getObject(i);
    print(object);
    print(
        'Worker: object[$i]: name: ${object.getName()}; class: ${object.getClassName()}');
  }

  /*
  AtumSceneObject object = scene.addObject("PhysBox");
  object.setName("_box01");
  object.trans = new Matrix4.identity();

  Map data = {};
  object.loadFromJson(JSON.encode(data));
  */

  print('Worker: ready');
  timer = new Timer.periodic(const Duration(milliseconds: 10), (_) => update());
}
