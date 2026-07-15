#pragma once

#include <filesystem>

#include "BinaryEngine/Asset/Asset.h"
#include "BinaryEngine/Asset/AssetTypes.h"
#include "BinaryEngine/Core/Math.h"

namespace BinaryEngine {

	class Renderer;

	enum class ShaderStage {
		Vertex,
		Fragment,
	};

	struct ShaderSpecification {
		ShaderStage stage{ ShaderStage::Vertex };
		std::uint32_t samplerCount{};
		std::uint32_t uniformBufferCount{};
	};

	class Shader : public Asset {
	public:
		Shader() = default;
		virtual ~Shader() override;

		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete;
		Shader(Shader&&);
		Shader& operator=(Shader&&);

		static AssetType GetStaticAssetType() { return AssetType::Shader; }
		virtual AssetType GetAssetType() const override { return GetStaticAssetType(); }
		void Load(const std::filesystem::path& filePath, const Renderer& renderer, const ShaderSpecification shaderSpecification);
		void* GetNativeShader() const { return m_Shader; }
		bool IsValid() const { return m_Shader != nullptr; }

	private:
		struct SDL_GPUDevice* m_Device{};
		struct SDL_GPUShader* m_Shader{};
	};

}