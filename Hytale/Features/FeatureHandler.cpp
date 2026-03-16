#include "FeatureHandler.h"

#include "../Menu/Menu.h"
#include "../Menu/Children/Tab.h"

#include "ActualFeatures/Speed.h"

void InitFeature(std::unique_ptr<Feature> feature, std::string tab) {
	feature->setCategory(tab);
	feature->CreateForcedKeybind();
	feature->Initialize();

	FeatureHandler::features.push_back(std::move(feature));
}

void FeatureHandler::Init() {
	InitFeature(std::make_unique<Speed>(), "Movement");

	Menu::mainComponent->AddChild(std::make_unique<Tab>("Movement", 200, 200));
	Menu::mainComponent->AddChild(std::make_unique<Tab>("Visuals", 410, 200));
}