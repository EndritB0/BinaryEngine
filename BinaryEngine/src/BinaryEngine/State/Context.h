#pragma once

namespace BinaryEngine {

	class Window;
	class Renderer;

	struct Context {
		Window& window;
		Renderer& renderer;
	};

}