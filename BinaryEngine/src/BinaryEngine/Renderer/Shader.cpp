#include "pch.h"
#include "BinaryEngine/Renderer/Shader.h"

#include "BinaryEngine/Renderer/Renderer.h"

namespace {

	static SDL_GPUShaderStage ConvertShaderStage(BinaryEngine::ShaderStage stage)
	{
		switch (stage)
		{
			case BinaryEngine::ShaderStage::Vertex: return SDL_GPU_SHADERSTAGE_VERTEX;
			case BinaryEngine::ShaderStage::Fragment: return SDL_GPU_SHADERSTAGE_FRAGMENT;
			default: return SDL_GPU_SHADERSTAGE_VERTEX;
		}
	}

}

namespace BinaryEngine {

	Shader::~Shader()
	{
		if (m_Shader && m_Device)
		{
			SDL_ReleaseGPUShader(m_Device, m_Shader);
			m_Shader = nullptr;
			CORE_INFO("[Shader] Shader Destroyed");
		}
	}

	Shader::Shader(Shader&& other)
		: m_Device(other.m_Device), m_Shader(other.m_Shader)
	{
		other.m_Device = nullptr;
		other.m_Shader = nullptr;
	}

	Shader& Shader::operator=(Shader&& other)
	{
		if (this != &other)
		{
			if (m_Shader && m_Device)
			{
				SDL_ReleaseGPUShader(m_Device, m_Shader);
			}

			m_Device = other.m_Device;
			m_Shader = other.m_Shader;

			other.m_Device = nullptr;
			other.m_Shader = nullptr;
		}

		return *this;
	}

	void Shader::Load(const std::filesystem::path& filePath, const Renderer& renderer, const ShaderSpecification shaderSpecification)
	{
		SDL_GPUDevice* device = static_cast<SDL_GPUDevice*>(renderer.GetNativeDevice());
		if (!device)
		{
			CORE_ERROR("[Shader] No GPU Device available to create shader: {}", filePath.string());
			return;
		}

		SDL_GPUShaderFormat supportedFormat = SDL_GetGPUShaderFormats(device);

		struct FormatOption { SDL_GPUShaderFormat format; const char* extension; };
		constexpr FormatOption s_SupportedShaderFormats[]{
			{ SDL_GPU_SHADERFORMAT_SPIRV, ".spv" },
			{ SDL_GPU_SHADERFORMAT_DXIL, ".dxil" },
			{ SDL_GPU_SHADERFORMAT_MSL, ".msl" },
		};

		const char* extension{ nullptr };

		SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_INVALID;
		for (const FormatOption& option : s_SupportedShaderFormats)
		{
			if (supportedFormat & option.format)
			{
				format = option.format;
				extension = option.extension;
				break;
			}
		}

		if (!extension)
		{
			CORE_ERROR("[Shader] No supported shader format found for device");
			return;
		}

		std::string path{ filePath.string() + extension };

		std::ifstream file(path, std::ios::binary | std::ios::ate);
		if (!file)
		{
			CORE_ERROR("[Shader] Failed to open shader file: {}", path);
			return;
		}

		const std::streamsize codeSize = file.tellg();
		file.seekg(0, std::ios::beg);

		std::vector<std::uint8_t> code(codeSize);
		if (!file.read(reinterpret_cast<char*>(code.data()), codeSize))
		{
			CORE_ERROR("[Shader] Failed to read shader file: {}", path);
			return;
		}

		SDL_GPUShaderCreateInfo info{
			.code_size {code.size()},
			.code {code.data()},
			.entrypoint {"main"},
			.format {format},
			.stage {ConvertShaderStage(shaderSpecification.stage)},
			.num_samplers {shaderSpecification.samplerCount},
			.num_storage_textures {0},
			.num_storage_buffers {0},
			.num_uniform_buffers {shaderSpecification.uniformBufferCount},
		};

		SDL_GPUShader* shader = SDL_CreateGPUShader(device, &info);
		if (!shader)
		{
			CORE_ERROR("[Shader] Failed to create Shader: {}, {}", path, SDL_GetError());
			return;
		}

		m_Device = device;
		m_Shader = shader;
		CORE_TRACE("[Shader] Shader Loaded: {}", path);
	}

}