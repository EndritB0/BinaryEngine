#pragma once

#include <cstdint>
#include <filesystem>

#include "BinaryEngine/Asset/Asset.h"
#include "BinaryEngine/Asset/AssetTypes.h"
#include "BinaryEngine/Core/Math.h"

struct SDL_GPUDevice;
struct SDL_GPUShader;

namespace BinaryEngine {

	class Renderer;

	enum class ShaderStage {
		Vertex,
		Fragment,
	};

	struct ShaderSpecification {
		ShaderStage Stage{ ShaderStage::Vertex };
		std::uint32_t SamplerCount{};
		std::uint32_t UniformBufferCount{};
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
		bool IsValid() const { return m_Shader != nullptr; }

	private:
		friend class Renderer;
		::SDL_GPUShader* GetNativeShader() const { return m_Shader; }

	private:
		::SDL_GPUDevice* m_Device{};
		::SDL_GPUShader* m_Shader{};
	};

}