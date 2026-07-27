#include "pch.h"
#include "BinaryEngine/Scene/Scene.h"

#include "BinaryEngine/Scene/Components.h"
#include "BinaryEngine/Scene/Entity.h"

namespace BinaryEngine {

	BinaryEngine::Scene::Scene()
	{
		CORE_INFO("[Scene] Scene Initialised");
	}

	Scene::~Scene()
	{
		CORE_INFO("[Scene] Scene Shutdown");
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity entity{ m_Registry.create(), this };
		entity.AddComponent<TagComponent>(name.empty() ? "Entity" : name);
		entity.AddComponent<TransformComponent>();
		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}

	void Scene::OnUpdate(TimeStep dt)
	{
		const float seconds{ dt.GetSeconds() };

		auto view = m_Registry.view<SpriteComponent, AnimationComponent>();
		view.each([seconds](SpriteComponent& sprite, AnimationComponent& anim) {
			const AnimationClip* clip{ anim.CurrentClip };
			if (!clip || clip->Frames.empty() || !anim.Playing || anim.Finished)
			{
				return;
			}

			const std::size_t frameCount{ clip->Frames.size() };

			if (clip->FrameDuration > 0.0f)
			{
				anim.ElapsedTime += seconds;
				while (anim.ElapsedTime >= clip->FrameDuration)
				{
					anim.ElapsedTime -= clip->FrameDuration;

					if (anim.CurrentFrame + 1 < frameCount)
					{
						anim.CurrentFrame++;
					}
					else if (clip->Looping)
					{
						anim.CurrentFrame = 0;
					}
					else
					{
						anim.CurrentFrame = frameCount - 1;
						anim.Finished = true;
						break;
					}
				}
			}

			const std::size_t safeFrame{ std::clamp(anim.CurrentFrame, std::size_t{ 0 }, frameCount - 1) };
			sprite.Region = clip->Frames[safeFrame];
			sprite.UseRegion = true;
				  });
	}

	void Scene::OnRender()
	{

	}

}