#include "pch.h"
#include "BinaryEngine/Renderer/OrthographicCamera.h"

#include "BinaryEngine/Renderer/CameraSpecification.h"

namespace {

	constexpr float s_MinimumZoom{ 0.0001f };

	bool IsPositiveSize(BinaryEngine::Vector2f size)
	{
		return size.x > 0.0f && size.y > 0.0f;
	}

	std::string_view CameraViewportModeToString(BinaryEngine::CameraViewportMode mode)
	{
		switch (mode)
		{
			case BinaryEngine::CameraViewportMode::Expand: return "Expand";
			case BinaryEngine::CameraViewportMode::FixedHeight: return "FixedHeight";
			case BinaryEngine::CameraViewportMode::FixedWidth: return "FixedWidth";
			case BinaryEngine::CameraViewportMode::Letterbox: return "Letterbox";
			case BinaryEngine::CameraViewportMode::Stretch: return "Stretch";
			default: return "Unknown";
		}
	}

}

namespace BinaryEngine {

	OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top)
	{
		BuildViewMatrix();
		SetOrthographicSize(Vector2f{ right - left, top - bottom });
	}

	OrthographicCamera::OrthographicCamera(Vector2i windowSize) :
		OrthographicCamera(windowSize,
						   Vector2f{ static_cast<float>(windowSize.x), static_cast<float>(windowSize.y) },
						   CameraViewportMode::FixedHeight)
	{}

	OrthographicCamera::OrthographicCamera(Vector2i windowSize, const CameraSpecification& specification) :
		OrthographicCamera(windowSize,
						   specification.DesignSize.value_or(Vector2f{ static_cast<float>(windowSize.x), static_cast<float>(windowSize.y) }),
						   specification.ViewportMode)
	{}

	OrthographicCamera::OrthographicCamera(Vector2i windowSize, Vector2f designSize, CameraViewportMode mode)
	{
		m_WindowSize = windowSize;
		m_ViewportMode = mode;

		if (IsPositiveSize(designSize))
		{
			m_DesignSize = designSize;
		}
		else
		{
			CORE_WARN("[OrthographicCamera] Design size must be positive on both axes, received {}x{}, falling back to {}x{}",
					  designSize.x, designSize.y, m_DesignSize.x, m_DesignSize.y);
		}

		BuildViewMatrix();
		BuildProjectionMatrix();
		RecalculateViewProjection();
	}

	void OrthographicCamera::SetOrthographicSize(Vector2f worldSize)
	{
		if (IsPositiveSize(worldSize))
		{
			m_DesignSize = worldSize;
		}
		else
		{
			CORE_WARN("[OrthographicCamera] Orthographic size must be positive on both axes, received {}x{}, falling back to {}x{}",
					  worldSize.x, worldSize.y, m_DesignSize.x, m_DesignSize.y);
		}

		m_ViewportMode = CameraViewportMode::Stretch;
		m_Zoom = 1.0f;
		RecalculateProjection();
	}

	void OrthographicCamera::OnResize(Vector2i windowSize)
	{
		m_WindowSize = windowSize;
		RecalculateProjection();
	}

	void OrthographicCamera::SetZoom(float zoom)
	{
		if (!(zoom > 0.0f))
		{
			CORE_WARN("[OrthographicCamera] Zoom must be greater than zero, received {}, clamping to {}", zoom, s_MinimumZoom);
			zoom = s_MinimumZoom;
		}

		m_Zoom = zoom;
		RecalculateProjection();
	}

	void OrthographicCamera::SetViewportMode(CameraViewportMode mode)
	{
		m_ViewportMode = mode;
		RecalculateProjection();
		CORE_TRACE("[OrthographicCamera] Viewport mode set to '{}'", CameraViewportModeToString(mode));
	}

	void OrthographicCamera::SetDesignSize(Vector2f designSize)
	{
		if (!IsPositiveSize(designSize))
		{
			CORE_WARN("[OrthographicCamera] Design size must be positive on both axes, received {}x{}, keeping {}x{}",
					  designSize.x, designSize.y, m_DesignSize.x, m_DesignSize.y);
			return;
		}

		m_DesignSize = designSize;
		RecalculateProjection();
	}

	CameraViewport OrthographicCamera::CalculateViewport(Vector2i targetSize) const
	{
		CameraViewport viewport{ { 0, 0 }, targetSize };

		if (m_ViewportMode != CameraViewportMode::Letterbox)
		{
			return viewport;
		}

		if (targetSize.x <= 0 || targetSize.y <= 0 || !IsPositiveSize(m_DesignSize))
		{
			return viewport;
		}

		const float designAspectRatio{ m_DesignSize.x / m_DesignSize.y };
		const float targetAspectRatio{ static_cast<float>(targetSize.x) / static_cast<float>(targetSize.y) };

		if (targetAspectRatio > designAspectRatio)
		{
			viewport.Size.y = targetSize.y;
			viewport.Size.x = static_cast<int>(static_cast<float>(targetSize.y) * designAspectRatio);
		}
		else
		{
			viewport.Size.x = targetSize.x;
			viewport.Size.y = static_cast<int>(static_cast<float>(targetSize.x) / designAspectRatio);
		}

		viewport.Position.x = (targetSize.x - viewport.Size.x) / 2;
		viewport.Position.y = (targetSize.y - viewport.Size.y) / 2;

		return viewport;
	}

	Vector2f OrthographicCamera::ScreenToWorld(Vector2f screenPosition, Vector2i windowSize) const
	{
		const CameraViewport viewport{ CalculateViewport(windowSize) };

		if (viewport.Size.x <= 0 || viewport.Size.y <= 0)
		{
			return Vector2f{ m_Position.x, m_Position.y };
		}

		const float normalisedX{ (screenPosition.x - static_cast<float>(viewport.Position.x)) / static_cast<float>(viewport.Size.x) };
		const float normalisedY{ (screenPosition.y - static_cast<float>(viewport.Position.y)) / static_cast<float>(viewport.Size.y) };

		const glm::vec4 clipPosition{ normalisedX * 2.0f - 1.0f, 1.0f - normalisedY * 2.0f, 0.0f, 1.0f };

		glm::vec4 worldPosition{ m_InverseViewProjectionMatrix * clipPosition };
		if (worldPosition.w != 0.0f)
		{
			worldPosition /= worldPosition.w;
		}

		return Vector2f{ worldPosition.x, worldPosition.y };
	}

	Vector2f OrthographicCamera::WorldToScreen(Vector2f worldPosition, Vector2i windowSize) const
	{
		const CameraViewport viewport{ CalculateViewport(windowSize) };

		glm::vec4 clipPosition{ m_ViewProjectionMatrix * glm::vec4{ worldPosition.x, worldPosition.y, 0.0f, 1.0f } };
		if (clipPosition.w != 0.0f)
		{
			clipPosition /= clipPosition.w;
		}

		const float normalisedX{ (clipPosition.x + 1.0f) * 0.5f };
		const float normalisedY{ (1.0f - clipPosition.y) * 0.5f };

		return Vector2f{
			static_cast<float>(viewport.Position.x) + normalisedX * static_cast<float>(viewport.Size.x),
			static_cast<float>(viewport.Position.y) + normalisedY * static_cast<float>(viewport.Size.y)
		};
	}

	void OrthographicCamera::RecalculateViewMatrix()
	{
		BuildViewMatrix();
		RecalculateViewProjection();
	}

	void OrthographicCamera::BuildViewMatrix()
	{
		glm::mat4 transform{ glm::translate(glm::mat4(1.0f), m_Position) * glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0, 0, 1)) };
		m_ViewMatrix = glm::inverse(transform);
	}

	void OrthographicCamera::RecalculateProjection()
	{
		BuildProjectionMatrix();
		RecalculateViewProjection();
	}

	void OrthographicCamera::BuildProjectionMatrix()
	{
		const float windowWidth{ static_cast<float>(m_WindowSize.x) };
		const float windowHeight{ static_cast<float>(m_WindowSize.y) };

		if (windowWidth <= 0.0f || windowHeight <= 0.0f)
		{
			return;
		}

		const float aspectRatio{ windowWidth / windowHeight };

		Vector2f viewSize{ m_DesignSize };
		switch (m_ViewportMode)
		{
			case CameraViewportMode::Expand:
			{
				viewSize = { windowWidth, windowHeight };
				break;
			}

			case CameraViewportMode::FixedHeight:
			{
				viewSize = { m_DesignSize.y * aspectRatio, m_DesignSize.y };
				break;
			}

			case CameraViewportMode::FixedWidth:
			{
				viewSize = { m_DesignSize.x, m_DesignSize.x / aspectRatio };
				break;
			}

			case CameraViewportMode::Letterbox:
			case CameraViewportMode::Stretch:
			{
				viewSize = m_DesignSize;
				break;
			}
		}

		viewSize /= m_Zoom;
		m_WorldViewSize = viewSize;

		const float halfWidth{ viewSize.x / 2.0f };
		const float halfHeight{ viewSize.y / 2.0f };

		m_ProjectionMatrix = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, -1.0f, 1.0f);
	}

	void OrthographicCamera::RecalculateViewProjection()
	{
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
		m_InverseViewProjectionMatrix = glm::inverse(m_ViewProjectionMatrix);
	}

}