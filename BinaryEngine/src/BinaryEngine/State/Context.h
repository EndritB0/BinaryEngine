#pragma once

namespace BinaryEngine {

	class Window;
	class Renderer;
	class AssetManager;
	class AudioEngine;
	struct CameraSpecification;

	struct Context {
		Window& window;
		Renderer& renderer;
		AssetManager& assetManager;
		AudioEngine& audio;
		const CameraSpecification& camera;
	};

}