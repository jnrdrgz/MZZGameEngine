#pragma once

namespace Helpers {
	struct Random {
		static int random_between(int mn, int mx) {
			if (mn == mx) return mn;
			int n = rand() % (mx - mn) + mn;
			return n;
		}

		static float random_betweenf(float min, float max) {
			float scale = rand() / (float)RAND_MAX; // [0, 1.0] 
			return min + scale * (max - min);      // [min, max]
		}
	};
}