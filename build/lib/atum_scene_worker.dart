import 'dart:isolate';
import 'dart:typed_data';

import 'package:atum/atum_extension.dart';

main(args, sendPort) {
  AtumCore core = new AtumCore();

  Isolate iso = Isolate.current;

  print('Scene Worker!!!');
  print(core);

  if (sendPort is SendPort) {
    sendPort.send(new Uint8List.fromList([1, 2, 3]));
  }
}
