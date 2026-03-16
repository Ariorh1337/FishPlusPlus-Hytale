#pragma once
#include "Feature.h"

namespace FeatureHandler {
	inline std::vector<std::unique_ptr<Feature>> features;

	void Init();
}
