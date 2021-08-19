#pragma once

class SimpleMath {
 public:

/*double abs(double a) {
  return a < 0 ? -a : a;
}*/

	static double min(double a, double b) {
		return a < b ? a : b;
	}
	static double max(double a, double b) {
		return a > b ? a : b;
	}

	static double clamp(double minimum, double maximum, double input) {
		return max(minimum, min(input, maximum));
	}

	static double approach(double target, double amt, double input) {
		return input > target ? max(target, input - amt) : min(target, input + amt);
	}
};
