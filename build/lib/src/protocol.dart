import 'package:crc32/crc32.dart';

/// TypeId = CRC32('ClassName')

const int _SceneControlTypeId = 0x7d684fde;

class _SceneControl {
  final int value;
  const _SceneControl(this.value);
}

class SceneControlPlay extends _SceneControl {
  const SceneControlPlay() : super((_SceneControlTypeId << 32) | 1);
}

class SceneControlStop extends _SceneControl {
  const SceneControlStop() : super((_SceneControlTypeId << 32) | 2);
}

class SceneControlUnload extends _SceneControl {
  const SceneControlUnload() : super((_SceneControlTypeId << 32) | 3);
}

_check() {
  assert(CRC32.compute('SceneControl') == _SceneControlTypeId);
}

serialize(data) {
  _check();

  if (data is _SceneControl) {
    return data.value;
  }

  return null;
}

deserialize(data) {
  _check();

  if (data == const SceneControlPlay().value) return const SceneControlPlay();
  if (data == const SceneControlStop().value) return const SceneControlStop();
  if (data == const SceneControlUnload().value)
    return const SceneControlUnload();

  return null;
}
