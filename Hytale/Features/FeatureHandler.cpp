#include "FeatureHandler.h"

#include "../Menu/Menu.h"
#include "../Menu/Children/Tab.h"

#include "ActualFeatures/Speed.h"
#include "ActualFeatures/Flight.h"
#include "ActualFeatures/NameTags.h"
#include "ActualFeatures/ESP.h"
#include "ActualFeatures/NoFall.h"
#include "ActualFeatures/WorldModulate.h"

void InitFeature(std::unique_ptr<Feature> feature, std::string tab) {
	feature->setCategory(tab);
	feature->CreateForcedKeybind();
	feature->Initialize();

	FeatureHandler::features.push_back(std::move(feature));
}

void FeatureHandler::Init() {
	InitFeature(std::make_unique<Flight>(), "Movement");
	InitFeature(std::make_unique<Speed>(), "Movement");
	InitFeature(std::make_unique<NoFall>(), "Movement");

	InitFeature(std::make_unique<ESP>(), "Visuals");
	InitFeature(std::make_unique<Nametags>(), "Visuals");
	InitFeature(std::make_unique<WorldModulate>(), "Visuals");

	Menu::mainComponent->AddChild(std::make_unique<Tab>("Movement", 200, 200));
	Menu::mainComponent->AddChild(std::make_unique<Tab>("Visuals", 410, 200));
}

Feature* FeatureHandler::GetFeatureFromName(std::string name) {
	for (auto& feature : features) {
		if (name == feature->GetName())
			return feature.get();
	}
	return nullptr;
}