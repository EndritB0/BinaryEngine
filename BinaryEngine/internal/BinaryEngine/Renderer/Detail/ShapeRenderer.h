#pragma once

#include <cstdint>
#include <vector>

#include "BinaryEngine/Core/Color.h"
#include "BinaryEngine/Core/Math.h"
#include "BinaryEngine/Renderer/Detail/QuadRenderer.h"
#include "BinaryEngine/Renderer/RendererSpecification.h"

struct SDL_GPUDevice;
struct SDL_GPUShader;
struct SDL_Window;

namespace BinaryEngine::Detail {

	enum class ShapeType : std::uint32_t {
		Rectangle = 0,
		Circle,
		Triangle
	};

	struct ShapeCommand {
		Vector2f Origin{ 0.0f, 0.0f };
		Vector2f EdgeX{ 0.0f, 0.0f };
		Vector2f EdgeY{ 0.0f, 0.0f };
		Vector2f HalfSize{ 0.0f, 0.0f };
		float Margin{ 0.0f };
		Color VertexColor{};
		float OutlineThickness{ 0.0f };
		ShapeType Type{ ShapeType::Rectangle };
	};

	class ShapeRenderer : public QuadRenderer {
	public:
		explicit ShapeRenderer(QuadIndexBuffer& indexBuffer);

		bool Initialise(::SDL_GPUDevice* device, ::SDL_Window* window, ::SDL_GPUShader* vertexShader, ::SDL_GPUShader* fragmentShader, const RendererSpecification& specification);

		std::uint32_t AddCommand(ShapeType shapeType, Vector2f center, Vector2f edgeX, Vector2f edgeY, Vector2f halfSize, float margin, Color vertexColor, float outlineThickness)
		{
			const Vector2f extendedEdgeX{ edgeX * (1.0f + margin / halfSize.x) };
			const Vector2f extendedEdgeY{ edgeY * (1.0f + margin / halfSize.y) };

			const std::uint32_t commandIndex{ static_cast<std::uint32_t>(m_Commands.size()) };
			m_Commands.emplace_back(center - (extendedEdgeX + extendedEdgeY) * 0.5f, extendedEdgeX, extendedEdgeY, halfSize, margin, vertexColor, outlineThickness, shapeType);
			return commandIndex;
		}

		virtual void ResetFrame() override;

	protected:
		virtual void WriteVertices(void* destination, const std::vector<std::uint32_t>& drawOrder, std::uint32_t quadCount) override;

	private:
		std::vector<ShapeCommand> m_Commands;
	};

}
