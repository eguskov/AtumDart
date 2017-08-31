// Copyright (c) 2012, the Dart project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

library atum;

import 'dart:async';
import 'dart:isolate';

import 'dart:typed_data';
import 'package:vector_math/vector_math.dart';

import 'dart-ext:AtumExt';

int systemRand() native "SystemRand";
int noScopeSystemRand() native "NoScopeSystemRand";
bool systemSrand(int seed) native "SystemSrand";

abstract class _Native extends NativeClass1 {
  void _new();
  void _setServicePort(SendPort port);
  SendPort _getServicePort();
  SendPort _newServicePort();
  SendPort get _servicePort {
    if (_getServicePort() == null) {
      _setServicePort(_newServicePort());
    }
    return _getServicePort();
  }
}

class _NativeWithoutServicePort extends _Native {
  @override
  SendPort _getServicePort() => null;

  @override
  SendPort _newServicePort() => null;

  @override
  void _setServicePort(SendPort) {}
}

class AtumCore extends _Native {
  static SendPort _port;

  AtumCore() {
    _new();
  }

  init() native "AtumCore::init";
  double update() native "AtumCore::update";
  int controlsGetAlias(String) native "AtumCore::controlsGetAlias";
  bool controlsIsDebugKeyActive(String) native "AtumCore::controlsIsDebugKeyActive";
  bool controlsIsDebugKeyPressed(String) native "AtumCore::controlsIsDebugKeyPressed";

  AtumScene _addScene(Type sceneType) native "AtumCore::addScene";
  AtumScene addScene() => _addScene(AtumScene);

  // Future<bool> init() {
  //   var completer = new Completer();
  //   var replyPort = new RawReceivePort();
  //   var args = new List(3);
  //   args[0] = 1;
  //   args[1] = _handle;
  //   args[2] = replyPort.sendPort;
  //   _servicePort.send(args);
  //   replyPort.handler = (result) {
  //     replyPort.close();
  //     if (result != null) {
  //       completer.complete(result);
  //     } else {
  //       completer.completeError(new Exception("AtumCore::Init failed"));
  //     }
  //   };
  //   return completer.future;
  // }

  // Future<bool> init() {
  //   var completer = new Completer();
  //   var replyPort = new RawReceivePort();
  //   var args = new List(3);
  //   args[0] = 1;
  //   args[1] = _handle;
  //   args[2] = replyPort.sendPort;
  //   _servicePort.send(args);
  //   replyPort.handler = (result) {
  //     replyPort.close();
  //     if (result != null) {
  //       completer.complete(result);
  //     } else {
  //       completer.completeError(new Exception("AtumCore::Init failed"));
  //     }
  //   };
  //   return completer.future;
  // }

  // Future<bool> update() {
  //   var completer = new Completer();
  //   var replyPort = new RawReceivePort();
  //   var args = new List(3);
  //   args[0] = 2;
  //   args[1] = _handle;
  //   args[2] = replyPort.sendPort;
  //   _servicePort.send(args);
  //   replyPort.handler = (result) {
  //     replyPort.close();
  //     if (result != null) {
  //       completer.complete(result);
  //     } else {
  //       completer.completeError(new Exception("AtumCore::Update failed"));
  //     }
  //   };
  //   return completer.future;
  // }

  // Future<bool> run() {
  //   var completer = new Completer();
  //   var replyPort = new RawReceivePort();
  //   var args = new List(3);
  //   args[0] = 3;
  //   args[1] = _handle;
  //   args[2] = replyPort.sendPort;
  //   _servicePort.send(args);
  //   replyPort.handler = (result) {
  //     replyPort.close();
  //     if (result != null) {
  //       completer.complete(result);
  //     } else {
  //       completer.completeError(new Exception("AtumCore::Run failed"));
  //     }
  //   };
  //   return completer.future;
  // }

  @override
  SendPort _getServicePort() => _port;

  @override
  _setServicePort(SendPort port) {
    _port = port;
  }

  @override
  void _new() native "AtumCore::AtumCore";

  @override
  SendPort _newServicePort() native "AtumCore::servicePort";
}

class AtumSceneObject extends _NativeWithoutServicePort {
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

class AtumScene extends _NativeWithoutServicePort {
  AtumSceneObject _addObject(Type sceneObjectType, String typeName) native "AtumScene::addObject";
  AtumSceneObjectaddObject(String typeName) => _addObject(AtumSceneObject, typeName);

  load(String path) native "AtumScene::load";
  play() native "AtumScene::play";

  AtumSceneObject _getObject(Type sceneObjectType, int index) native "AtumScene::getObject";
  AtumSceneObject getObject(int index) => _getObject(AtumSceneObject, index);

  int getObjectsCount() native "AtumScene::getObjectsCount";

  @override
  void _new() native "AtumScene::AtumScene";
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
