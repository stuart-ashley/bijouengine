#pragma once

namespace render {
	enum class BlendFlag {
		NONE,
		/** destination color = source color + destination color */
		ADDITIVE,
		/** destination color = source color * source alpha + destination color */
		SRC_ALPHA_ADDITIVE,
		/** destination color = source color * source alpha + destination color * ( 1 - source alpha ) */
		SRC_ALPHA
	};
}

