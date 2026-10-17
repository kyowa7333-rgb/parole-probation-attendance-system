import 'package:firebase_core/firebase_core.dart';
import 'package:last/firebase_options.dart';

/// True after running `flutterfire configure` with real project values.
class FirebaseConfig {
  static bool get isConfigured {
    final projectId = DefaultFirebaseOptions.currentPlatform.projectId;
    return projectId.isNotEmpty && !projectId.startsWith('YOUR_');
  }

  static bool get canUseFirestore =>
      isConfigured && Firebase.apps.isNotEmpty;
}
