// Copyright (c) 2012, the Dart project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

library atum;

import 'dart:io';
import 'dart:async';
import 'dart:isolate';

import 'dart:typed_data';
import 'package:vector_math/vector_math.dart';

import 'dart-ext:AtumExt';

import 'src/protocol.dart' as proto;

// TODO: Generate gen.dart with list of files
part 'SceneObject.gen.dart';
part 'PhysBox.gen.dart';
part 'Scene.gen.dart';
part 'Terrain.gen.dart';

int systemRand() native "SystemRand";
int noScopeSystemRand() native "NoScopeSystemRand";
bool systemSrand(int seed) native "SystemSrand";

class _Native extends NativeClass1 {
  void _new() {}
}

class IsolateConnection {
  final ReceivePort portOnExit = new ReceivePort();
  final ReceivePort portOnError = new ReceivePort();
  final ReceivePort portOnMessage = new ReceivePort();

  Isolate isolate;
  SendPort peerPort;

  IsolateConnection() {
    portOnExit.listen((message) {
      print('portOnExit: ${message}');
    });

    portOnError.listen((message) {
      print('portOnError: ${message}');
    });

    portOnMessage.listen((message) {
      print('portOnMessage: ${message}');
      if (message is SendPort) {
        peerPort = message;
      }
    });
  }

  Future<bool> open(Uri uri) async {
    isolate = await Isolate.spawnUri(uri, [portOnMessage.sendPort], null,
        onExit: portOnExit.sendPort, onError: portOnError.sendPort);

    if (isolate == null) {
      return false;
    }

    var completer = new Completer();

    Timer waitTimer;
    waitTimer = new Timer.periodic(const Duration(milliseconds: 10), (_) {
      if (peerPort != null) {
        waitTimer.cancel();
        completer.complete(true);
      }
    });

    return completer.future;
  }

  send(message) async {
    peerPort.send(proto.serialize(message));
  }
}

class AtumCore extends _Native {
  static SendPort _port;

  AtumCore() {
    _new();
  }

  init() native "AtumCore::init";
  double update() native "AtumCore::update";
  int controlsGetAlias(String) native "AtumCore::controlsGetAlias";
  bool controlsIsDebugKeyActive(String key)
      native "AtumCore::controlsIsDebugKeyActive";
  bool controlsIsDebugKeyPressed(String key)
      native "AtumCore::controlsIsDebugKeyPressed";
  double controlsGetAliasValue(int alias, bool delta)
      native "AtumCore::controlsGetAliasValue";
  double renderGetWidth() native "AtumCore::renderGetWidth";
  double renderGetHeight() native "AtumCore::renderGetHeight";

  // AtumScene _addScene(Type sceneType) native "AtumCore::addScene";
  // AtumScene addScene() => _addScene(AtumScene);
  addScene(SceneObject scene) native "AtumCore::addScene";

  Future<IsolateConnection> loadSceneIsolate(String path) async {
    var conn = new IsolateConnection();
    bool res = await conn.open(new Uri.file(path));
    return res ? conn : null;
  }

  SendPort get _servicePort {
    if (_port == null) {
      _port = _newServicePort();
    }
    return _port;
  }

  @override
  void _new() native "AtumCore::AtumCore";

  SendPort _newServicePort() native "AtumCore::servicePort";
}

class AtumSceneObject extends _Native {
  Float32List _getTrans() native "AtumSceneObject::getTrans";
  Matrix4 get trans => new Matrix4.fromFloat32List(_getTrans());

  _setTrans(Float32List m) native "AtumSceneObject::setTrans";
  void set trans(Matrix4 m) => _setTrans(m.storage);

  String getName() native "AtumSceneObject::getName";
  String getClassName() native "AtumSceneObject::getClassName";

  cast(Type type) native "AtumSceneObject::cast";

  @override
  void _new() native "AtumSceneObject::AtumSceneObject";
}

class AtumTank extends AtumSceneObject {
  Float32List _getAngles() native "AtumTank::getAngles";
  Vector3 get angles => new Vector3.fromFloat32List(_getAngles());

  _setAngles(Float32List) native "AtumTank::setAngles";
  void set angles(Vector3 v) => _setAngles(v.storage);

  _move(Float32List m) native "AtumTank::move";
  void move(Vector3 v) => _move(v.storage);

  @override
  void _new() native "AtumTank::AtumTank";
}

class AtumScene extends _Native {
  List<AtumSceneObject> _objectsCache = [];

  AtumSceneObject _addObject(Type sceneObjectType, String typeName)
      native "AtumScene::addObject";
  AtumSceneObject addObject(String typeName, {Type type = AtumSceneObject}) =>
      _addObject(type, typeName);

  load(String path) native "AtumScene::load";
  play() native "AtumScene::play";

  AtumSceneObject _getObject(Type sceneObjectType, int index)
      native "AtumScene::getObject";
  AtumSceneObject getObject(int index) {
    if (index >= _objectsCache.length) {
      _objectsCache.length = index + 1;
    }

    if (_objectsCache[index] == null) {
      _objectsCache[index] = _getObject(AtumSceneObject, index);
      if (_objectsCache[index].getClassName() == 'Tank') {
        _objectsCache[index] = _objectsCache[index].cast(AtumTank);
      }
    }

    return _objectsCache[index];
  }

  int getObjectsCount() native "AtumScene::getObjectsCount";

  @override
  void _new() native "AtumScene::AtumScene";
}

class RayCastDesc {
  Vector origin;
  Vector dir;
  Vector hitPos;
  Vector hitNormal;

  double length;
}

class AtumPhysScene extends _Native {
  bool rayCast(RayCastDesc desc) native "AtumPhysScene::rayCast";

  @override
  void _new() native "AtumPhysScene::AtumPhysScene";
}

// A class caches the native port used to call an asynchronous extension.
class RandomArray {
  static SendPort _port;

  Future<List<int>> randomArray(int seed, int length) {
    var completer = new Completer();
    var replyPort = new RawReceivePort();
    var args = new List(3);
    args[0] = seed;
    args[1] = length;
    args[2] = replyPort.sendPort;
    _servicePort.send(args);
    replyPort.handler = (result) {
      replyPort.close();
      if (result != null) {
        completer.complete(result);
      } else {
        completer.completeError(new Exception("Random array creation failed"));
      }
    };
    return completer.future;
  }

  SendPort get _servicePort {
    if (_port == null) {
      _port = _newServicePort();
    }
    return _port;
  }

  SendPort _newServicePort() native "RandomArray_ServicePort";
}
