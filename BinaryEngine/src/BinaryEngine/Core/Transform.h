#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "BinaryEngine/Core/Math.h"

namespace BinaryEngine {

	struct Transform {
		Vector3f Position{ 0.0f, 0.0f, 0.0f };
		Vector3f Rotation{ 0.0f, 0.0f, 0.0f };
		Vector3f Scale{ 1.0f, 1.0f, 1.0f };

		Transform(const Vector3f& position, const  Vector3f rotation, const  Vector3f scale) :
			Position(position), Rotation(rotation), Scale(scale)
		{
		};

		glm::mat4 GetModelMatrix() const
		{
			glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), Rotation.x, { 1, 0, 0 }) *
				glm::rotate(glm::mat4(1.0f), Rotation.y, { 0, 1, 0 }) *
				glm::rotate(glm::mat4(1.0f), Rotation.z, { 0, 0, 1 });

			return glm::translate(glm::mat4(1.f), Position) * rotationMatrix * glm::scale(glm::mat4(1.0f), Scale);
		}

	};

}