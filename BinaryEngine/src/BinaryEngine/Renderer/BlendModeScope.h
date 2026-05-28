#pragma once

#include "BinaryEngine/Renderer/BlendMode.h"

namespace BinaryEngine {

	class Renderer;

	class BlendModeScope {
	public:
		BlendModeScope(Renderer& renderer, BlendMode Mode);
		~BlendModeScope();

		BlendModeScope(const BlendModeScope&) = delete;
		BlendModeScope& operator=(const BlendModeScope&) = delete;
		BlendModeScope(BlendModeScope&&) = delete;
		BlendModeScope& operator=(BlendModeScope&&) = delete;

	private:
		Renderer& m_Renderer;
		BlendMode m_PreviousBlendMode;
	};

}