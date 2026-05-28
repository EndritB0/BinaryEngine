#include "pch.h"
#include "BinaryEngine/Renderer/BlendModeScope.h"

#include "BinaryEngine/Renderer/Renderer.h"

namespace BinaryEngine {

	BlendModeScope::BlendModeScope(Renderer& renderer, BlendMode Mode)
		: m_Renderer(renderer)
	{
		m_PreviousBlendMode = m_Renderer.GetBlendMode();
		m_Renderer.SetBlendMode(Mode);
	}

	BlendModeScope::~BlendModeScope()
	{
		m_Renderer.SetBlendMode(m_PreviousBlendMode);
	}

}