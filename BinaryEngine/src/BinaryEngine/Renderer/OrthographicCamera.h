#pragma once

#include <glm/fwd.hpp>

#include "BinaryEngine/Core/Math.h"
#include "BinaryEngine/Renderer/CameraSpecification.h"

namespace BinaryEngine {

	class OrthographicCamera {
	public:
		OrthographicCamera(Vector2i windowSize, Vector2f designSize, CameraViewportMode mode = CameraViewportMode::FixedHeight);
		OrthographicCamera(Vector2i windowSize, const CameraSpecification& specification);

		void OnResize(Vector2i windowSize);
		const Vector3f& GetPosition() const { return m_Position; }
		void SetPosition(const Vector3f& position) { m_Position = position; RecalculateViewMatrix(); }
		float GetRotation() const { return m_Rotation; }
		void SetRotation(float rotation);
		float GetZoom() const { return m_Zoom; }
		void SetZoom(float zoom);
		CameraViewportMode GetViewportMode() const { return m_ViewportMode; }
		void SetViewportMode(CameraViewportMode mode);
		CameraSnapMode GetSnapMode() const { return m_SnapMode; }
		void SetSnapMode(CameraSnapMode mode);
		float GetPixelsPerWorldUnit() const { return m_PixelsPerWorldUnit; }
		Vector2f SnapToPixelGrid(Vector2f worldPosition) const;
		Vector2f GetDesignSize() const { return m_DesignSize; }
		void SetDesignSize(Vector2f designSize);
		Vector2f GetWorldViewSize() const { return m_WorldViewSize; }
		Vector2f GetScreenViewSize() const { return CalculateViewSize(m_WindowSize); }
		Vector2i GetWindowSize() const { return m_WindowSize; }
		CameraViewport CalculateViewport(Vector2i targetSize) const;
		Vector2f ScreenToWorld(Vector2f screenPosition) const;
		Vector2f ScreenToWorld(Vector2f screenPosition, Vector2i windowSize) const;
		Vector2f WorldToScreen(Vector2f worldPosition) const;
		Vector2f WorldToScreen(Vector2f worldPosition, Vector2i windowSize) const;

		const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }
		const glm::mat4& GetInverseViewProjectionMatrix() const { return m_InverseViewProjectionMatrix; }

	private:
		Vector2f CalculateViewSize(Vector2i windowSize) const;
		void BuildViewMatrix();
		void BuildProjectionMatrix();
		void RecalculateViewMatrix();
		void RecalculateProjection();
		void RecalculateViewProjection();
		bool IsSnappingActive() const;
		void CheckSnappingUnsupported() const;

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

		CameraSnapMode m_SnapMode{ CameraSnapMode::None };
		float m_PixelsPerWorldUnit{ 1.0f };
		bool m_SnapRotationWarned{ false };

	};

}