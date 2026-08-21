#include "pch.h"
#include "BinaryEngine/Scene/Scene.h"

#include "BinaryEngine/Asset/AssetManager.h"
#include "BinaryEngine/Renderer/Font.h"
#include "BinaryEngine/Renderer/Renderer.h"
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

		auto animatedSpriteView = m_Registry.view<SpriteComponent, AnimationComponent>();
		animatedSpriteView.each([seconds](SpriteComponent& sprite, AnimationComponent& animation) {
			const AnimationClip* clip{ animation.CurrentClip };
			if (!clip || clip->Frames.empty() || !animation.Playing || animation.Finished)
			{
				return;
			}

			const std::size_t frameCount{ clip->Frames.size() };

			if (clip->FrameDuration > 0.0f)
			{
				animation.ElapsedTime += seconds;
				while (animation.ElapsedTime >= clip->FrameDuration)
				{
					animation.ElapsedTime -= clip->FrameDuration;

					if (animation.CurrentFrame + 1 < frameCount)
					{
						animation.CurrentFrame++;
					}
					else if (clip->Looping)
					{
						animation.CurrentFrame = 0;
					}
					else
					{
						animation.CurrentFrame = frameCount - 1;
						animation.Finished = true;
						break;
					}
				}
			}

			const std::size_t safeFrame{ std::clamp(animation.CurrentFrame, std::size_t{ 0 }, frameCount - 1) };
			sprite.Region = clip->Frames[safeFrame];
			sprite.UseRegion = true;
								});
	}

	void Scene::OnRender(Renderer& renderer, AssetManager& assetManager)
	{
		auto spriteView = GetAllEntitiesWith<TransformComponent, SpriteComponent>();

		spriteView.each([&](auto& transform, auto& sprite) {
			auto textureAsset = assetManager.GetAsset<Texture2D>(sprite.TextureHandle);

			if (!textureAsset)
			{
				return;
			}

			if (sprite.UseRegion)
			{
				renderer.DrawSprite(*textureAsset, transform.transform, sprite.Region);
			}
			else
			{
				renderer.DrawSprite(*textureAsset, transform.transform);
			}
						});

		auto textView = GetAllEntitiesWith<TransformComponent, TextComponent>();

		textView.each([&](auto& transform, auto& text) {
			auto fontAsset = assetManager.GetAsset<Font>(text.FontHandle);

			if (!fontAsset)
			{
				return;
			}

			renderer.DrawText(*fontAsset, text.Text, transform.transform, text.Specification);
					  });
	}

}