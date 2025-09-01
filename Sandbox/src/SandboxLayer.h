#pragma once

#include "Core/Layer.h"

#include "Renderer/Renderer.h"

class SandboxLayer : public Engine::Layer
{
public:
	SandboxLayer();
	virtual ~SandboxLayer() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	virtual void OnUpdate(float deltaTime) override;
	virtual void OnImGuiRender() override;

private:

};