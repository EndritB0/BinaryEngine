#pragma once

#include <cstdint>
#include <vector>

struct SDL_GPUCopyPass;
struct SDL_GPUDevice;
struct SDL_GPURenderPass;

namespace BinaryEngine::Detail {

	struct RenderBatch;

	class BatchRenderer {
	public:
		BatchRenderer() = default;
		virtual ~BatchRenderer() = default;

		BatchRenderer(const BatchRenderer&) = delete;
		BatchRenderer& operator=(const BatchRenderer&) = delete;
		BatchRenderer(BatchRenderer&&) = delete;
		BatchRenderer& operator=(BatchRenderer&&) = delete;

		virtual bool IsValid() const = 0;
		virtual void Release(::SDL_GPUDevice* device) = 0;
		virtual void ResetFrame() = 0;
		virtual void PrepareFrame(::SDL_GPUDevice* device, const std::vector<std::uint32_t>& drawOrder) = 0;
		virtual void Upload(::SDL_GPUDevice* device, ::SDL_GPUCopyPass* copyPass) = 0;
		virtual void Bind(::SDL_GPURenderPass* renderPass) = 0;
		virtual void DrawBatch(::SDL_GPURenderPass* renderPass, const RenderBatch& batch) = 0;
	};

}
