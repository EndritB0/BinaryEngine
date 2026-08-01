#pragma once

#include <glm/fwd.hpp>

#include "BinaryEngine/Core/Math.h"

namespace BinaryEngine {

	struct CameraSpecification;

	enum class CameraViewportMode {
		Expand,
		FixedHeight,
		FixedWidth,
		Letterbox,
		Stretch
	};

	struct CameraViewport {
		Vector2i Position{ 0, 0 };
		Vector2i Size{ 0, 0 };
	};

	class OrthographicCamera {
	public:
		OrthographicCamera(float left, float right, float bottom, float top);
		OrthographicCamera(Vector2i windowSize);
		OrthographicCamera(Vector2i windowSize, Vector2f designSize, CameraViewportMode mode = CameraViewportMode::FixedHeight);
		OrthographicCamera(Vector2i windowSize, const CameraSpecification& specification);

		void SetOrthographicSize(Vector2f worldSize);
		void OnResize(Vector2i windowSize);
		const Vector3f& GetPosition() const { return m_Position; }
		void SetPosition(const Vector3f& position) { m_Position = position; RecalculateViewMatrix(); }
		float GetRotation() const { return m_Rotation; }
		void SetRotation(float rotation) { m_Rotation = rotation; RecalculateViewMatrix(); }

		float GetZoom() const { return m_Zoom; }
		void SetZoom(float zoom);
		CameraViewportMode GetViewportMode() const { return m_ViewportMode; }
		void SetViewportMode(CameraViewportMode mode);
		Vector2f GetDesignSize() const { return m_DesignSize; }
		void SetDesignSize(Vector2f designSize);
		Vector2f GetWorldViewSize() const { return m_WorldViewSize; }
		Vector2i GetWindowSize() const { return m_WindowSize; }
		CameraViewport CalculateViewport(Vector2i targetSize) const;
		Vector2f ScreenToWorld(Vector2f screenPosition, Vector2i windowSize) const;
		Vector2f WorldToScreen(Vector2f worldPosition, Vector2i windowSize) const;

		const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }
		const glm::mat4& GetInverseViewProjectionMatrix() const { return m_InverseViewProjectionMatrix; }

	private:
		void BuildViewMatrix();
		void BuildProjectionMatrix();
		void RecalculateViewMatrix();
		void RecalculateProjection();
		void RecalculateViewProjection();

	private:
		glm::mat4 m_ProjectionMatrix{ 1.0f };
		glm::mat4 m_ViewMatrix{ 1.0f };
		glm::mat4 m_ViewProjectionMatrix{ 1.0f };
		glm::mat4 m_InverseViewProjectionMatrix{ 1.0f };

		Vector3f m_Position{ 0.0f, 0.0f, 0.0f };
		float m_Rotation{ 0.0f };

		Vector2f m_DesignSize{ 1280.0f, 720.0f };
		Vector2f m_WorldViewSize{ 1280.0f, 720.0f };
		Vector2i m_WindowSize{ 1280, 720 };
		CameraViewportMode m_ViewportMode{ CameraViewportMode::FixedHeight };
		float m_Zoom{ 1.0f };

	};

}