import 'package:last/core/utils/firebase_config.dart';
import 'package:last/features/tracking/services/firebase_tracking_repository.dart';
import 'package:last/features/tracking/services/mock_tracking_repository.dart';
import 'package:last/features/tracking/services/tracking_repository.dart';

TrackingRepository createTrackingRepository() {
  if (FirebaseConfig.canUseFirestore) {
    return FirebaseTrackingRepository();
  }
  return MockTrackingRepository();
}
