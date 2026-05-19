#pragma once

#include "BinaryEngine/Core/Math.h"

namespace BinaryEngine {

	class OrthographicCamera {
	public:
		OrthographicCamera(float left, float right, float bottom, float top);
		OrthographicCamera(Vector2i windowSize);

		void SetProjection(float left, float right, float bottom, float top);
		const Vector3f& GetPosition() const { return m_Position; }
		void SetPosition(const Vector3f& position) { m_Position = position; RecalculateViewMatrix(); }
		float GetRotation() const { return m_Rotation; }
		void SetRotation(float rotation) { m_Rotation = rotation; RecalculateViewMatrix(); }
		const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }

	private:
		void RecalculateViewMatrix();

	private:
		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ViewProjectionMatrix;

		Vector3f m_Position{ 0.0f, 0.0f, 0.0f };
		float m_Rotation{ 0.0f };

	};

}