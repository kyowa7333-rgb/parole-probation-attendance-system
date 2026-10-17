import 'dart:math';

import 'package:last/core/constants/app_constants.dart';

class TicketGenerator {
  static final Random _random = Random();

  static String generate() {
    final seq = (_random.nextInt(9000) + 1000).toString();
    return '${AppConstants.ticketPrefix}-${AppConstants.ticketYear}-$seq';
  }
}
