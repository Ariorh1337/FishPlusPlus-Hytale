/*
 * Copyright (c) FishPlusPlus.
 */

#include "BlockSelectionButton.h"

#include "../Style.h"
#include "../../Renderer/Renderer2D.h"
#include "../../Renderer/FontRenderer/Fonts.h"

BlockSelectionButton::BlockSelectionButton(Setting<std::vector<ClientBlockType*>>* setting) : SettingButton(setting) {
	auto body = std::make_unique<BlockSelectionWindow>(this);
	this->body = body.get();
	AddChild(std::move(body));
}

void BlockSelectionButton::Render(double deltaTime) {
	double fastDeltaTime = deltaTime * 20.0;

	m_hoverAlpha += (m_hovered ? 50.0f : -50.0f) * fastDeltaTime;
	if (m_hoverAlpha > Style::moduleHoverColor.a)
		m_hoverAlpha = Style::moduleHoverColor.a;
	if (m_hoverAlpha < 0)
		m_hoverAlpha = 0;

	auto* s = static_cast<Setting<std::vector<ClientBlockType*>>*>(this->setting);

	Renderer2D::colored->Square(Vector2(x, y), width, height, Color::Normalize(Style::moduleHoverColor.r, Style::moduleHoverColor.g, Style::moduleHoverColor.b, m_hoverAlpha));
	Renderer2D::colored->Render();

	Fonts::Figtree->RenderText(s->GetName(), x + Style::settingsNamePadding.x, y + Style::settingsNamePadding.y, 1.0f, Color::Normalize(Color::White()));
	Fonts::Figtree->RenderText(">", x + width - Fonts::Figtree->getWidth(">") - 6.0f, y + Style::settingsNamePadding.y, 1.0f, Color::Normalize(Color::White()));

	if (this->body->open)
		Component::Render(deltaTime);
}

void BlockSelectionButton::Update(float mouseX, float mouseY) {
	m_hovered = this->IsHovered(mouseX, mouseY);
	this->height = Style::featureHeight;
	Component::Update(mouseX, mouseY);
}

void BlockSelectionButton::MouseClicked(float mouseX, float mouseY, int vk) {
	if (this->body->open)
		Component::MouseClicked(mouseX, mouseY, vk);
	if (!this->IsHovered(mouseX, mouseY))
		return;

	if (vk == VK_RBUTTON || vk == VK_LBUTTON)
		this->body->open = !this->body->open;
}

BlockSelectionWindow::BlockSelectionWindow(BlockSelectionButton* parent) {
	this->parent = parent;
	this->x = parent->x + parent->width;
	this->y = parent->y;
}

void BlockSelectionWindow::Render(double deltaTime) {
	Renderer2D::colored->Square(Vector2(x, y), width, height, Color::Normalize(Style::tabBgColor));
	Renderer2D::colored->Render();
}

void BlockSelectionWindow::Update(float mouseX, float mouseY) {
	this->height = 500;
	this->width = 300;
}