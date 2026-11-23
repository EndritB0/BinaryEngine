#include "pch.h"
#include "Sandbox/States/QueriesDemoState.h"

#include <BinaryEngine/Core/Color.h>
#include <BinaryEngine/Core/FrameClock.h>
#include <BinaryEngine/Core/Transform.h>
#include <BinaryEngine/Input/Input.h>
#include <BinaryEngine/Physics/PhysicsBody.h>
#include <BinaryEngine/Physics/PhysicsWorld.h>
#include <BinaryEngine/Renderer/Font.h>
#include <BinaryEngine/Renderer/TextSpecification.h>
#include <BinaryEngine/Renderer/Texture2D.h>
#include <BinaryEngine/Scene/Components.h>

namespace Sandbox {

	namespace {

		constexpr std::string_view s_FontPath{ "./resources/fonts/OpenSans-Regular.ttf" };
		constexpr std::string_view s_QueryKeyHelp{ "1 ray   2 ray-all   3 shape cast   4 overlap   S shape   R rotate   E move the eye to the cursor   T triggers" };
		constexpr std::string_view s_KeyHelp{ "Left mouse  pick   Right mouse  capsule   X clear   C outlines" };
		constexpr BinaryEngine::Color s_ClearColor{ 0x12, 0x33, 0x38 };
		constexpr BinaryEngine::Color s_CapsuleColor{ 0xFF, 0x8C, 0x1A };
		constexpr BinaryEngine::Color s_TriggerZoneColor{ 0xFF, 0x50, 0xFF, 0x40 };
		constexpr BinaryEngine::Color s_EyeColor{ 0xFF, 0xFF, 0xFF };
		constexpr BinaryEngine::Color s_RayColor{ 0xFF, 0xE0, 0x40 };
		constexpr BinaryEngine::Color s_HitColor{ 0xFF, 0x40, 0x40 };
		constexpr BinaryEngine::Color s_NormalColor{ 0x40, 0xC0, 0xFF };
		constexpr BinaryEngine::Color s_GhostColor{ 0xC0, 0xC0, 0xC0 };
		constexpr BinaryEngine::Color s_HighlightColor{ 0xFF, 0xFF, 0x60 };
		constexpr float s_HudTextSize{ 12.0f };
		constexpr float s_HudZ{ 10.0f };
		constexpr float s_QueryZ{ 6.0f };
		constexpr float s_HighlightZ{ 5.5f };
		constexpr float s_BodyZ{ 0.0f };
		constexpr float s_TriggerZoneZ{ -0.5f };
		constexpr float s_QueryLineThickness{ 1.5f };
		constexpr float s_HighlightThickness{ 2.5f };
		constexpr float s_EyeSize{ 8.0f };
		constexpr float s_HitDotSize{ 6.0f };
		constexpr float s_NormalLength{ 20.0f };
		constexpr float s_MarkerTextSize{ 14.0f };
		constexpr float s_MarkerLabelOffset{ 14.0f };
		constexpr float s_QueryCircleRadius{ 16.0f };
		constexpr float s_RotationStepDegrees{ 15.0f };
		constexpr float s_CrateSize{ 32.0f };
		constexpr float s_BallRadius{ 16.0f };
		constexpr float s_SlopeRotationDegrees{ 20.0f };
		constexpr float s_SlopeFriction{ 0.05f };
		const BinaryEngine::Vector2f s_EyeStart{ -260.0f, -140.0f };
		const BinaryEngine::Vector2f s_QueryBoxSize{ 32.0f, 32.0f };
		const BinaryEngine::Vector2f s_GroundCenter{ 0.0f, -170.0f };
		const BinaryEngine::Vector2f s_GroundSize{ 640.0f, 20.0f };
		const BinaryEngine::Vector2f s_CeilingCenter{ 0.0f, 190.0f };
		const BinaryEngine::Vector2f s_LeftWallCenter{ -310.0f, 10.0f };
		const BinaryEngine::Vector2f s_RightWallCenter{ 310.0f, 10.0f };
		const BinaryEngine::Vector2f s_WallSize{ 20.0f, 380.0f };
		const BinaryEngine::Vector2f s_SlopeCenter{ -170.0f, 50.0f };
		const BinaryEngine::Vector2f s_SlopeSize{ 160.0f, 10.0f };
		const BinaryEngine::Vector2f s_PlatformCenter{ 170.0f, 60.0f };
		const BinaryEngine::Vector2f s_PlatformSize{ 140.0f, 10.0f };
		const BinaryEngine::Vector2f s_TriggerZoneCenter{ -110.0f, -110.0f };
		const BinaryEngine::Vector2f s_TriggerZoneSize{ 40.0f, 90.0f };
		const BinaryEngine::Vector2f s_LogCenter{ 170.0f, 77.0f };
		const BinaryEngine::Vector2f s_LogSize{ 64.0f, 24.0f };
		const BinaryEngine::Vector2f s_StandingCapsuleCenter{ 150.0f, -128.0f };
		const BinaryEngine::Vector2f s_CapsuleSize{ 32.0f, 64.0f };
		const std::array<BinaryEngine::Vector2f, 4> s_CratePositions{
			BinaryEngine::Vector2f{ -40.0f, -144.0f },
			BinaryEngine::Vector2f{ 0.0f, -144.0f },
			BinaryEngine::Vector2f{ 40.0f, -144.0f },
			BinaryEngine::Vector2f{ 80.0f, -144.0f },
		};
		const std::array<BinaryEngine::Vector2f, 3> s_BallPositions{
			BinaryEngine::Vector2f{ 205.0f, -144.0f },
			BinaryEngine::Vector2f{ 245.0f, -144.0f },
			BinaryEngine::Vector2f{ 222.0f, 81.0f },
		};

		std::string_view GetModeName(QueryMode mode)
		{
			switch (mode)
			{
				case QueryMode::Ray: return "ray";
				case QueryMode::RayAll: return "ray-all";
				case QueryMode::ShapeCast: return "shape cast";
				case QueryMode::Overlap: return "overlap";
			}

			return "unknown";
		}

		std::string_view GetShapeName(QueryShapeChoice choice)
		{
			switch (choice)
			{
				case QueryShapeChoice::Circle: return "circle";
				case QueryShapeChoice::Box: return "box";
				case QueryShapeChoice::Capsule: return "capsule";
			}

			return "unknown";
		}

		QueryShapeChoice GetNextShapeChoice(QueryShapeChoice choice)
		{
			switch (choice)
			{
				case QueryShapeChoice::Circle: return QueryShapeChoice::Box;
				case QueryShapeChoice::Box: return QueryShapeChoice::Capsule;
				case QueryShapeChoice::Capsule: return QueryShapeChoice::Circle;
			}

			return QueryShapeChoice::Circle;
		}

		std::string_view GetTag(BinaryEngine::Entity entity)
		{
			return entity.GetComponent<BinaryEngine::TagComponent>().Tag;
		}

		std::string DescribeCastHit(const std::optional<BinaryEngine::CastHit>& hit)
		{
			if (!hit)
			{
				return "No hit";
			}

			return std::format("Hit '{}'   fraction {:.3f}   point ({:.1f}, {:.1f})   normal ({:.2f}, {:.2f})", GetTag(hit->Entity), hit->Fraction, hit->Point.x, hit->Point.y, hit->Normal.x, hit->Normal.y);
		}

		void DrawCastHit(BinaryEngine::Renderer& renderer, const BinaryEngine::CastHit& hit)
		{
			renderer.DrawCircle(BinaryEngine::Transform{ { hit.Point.x, hit.Point.y, s_QueryZ }, { 0.0f, 0.0f, 0.0f }, { s_HitDotSize, s_HitDotSize, 1.0f } }, { .Color{ s_HitColor } });
			renderer.DrawLine(hit.Point, hit.Point + hit.Normal * s_NormalLength, { .Color{ s_NormalColor }, .Thickness{ s_QueryLineThickness }, .SortZ{ s_QueryZ } });
		}

		void DrawQueryShape(BinaryEngine::Renderer& renderer, const BinaryEngine::QueryShape& shape, BinaryEngine::Color color)
		{
			const BinaryEngine::Vector2f center{ shape.GetCenter() };
			const BinaryEngine::Vector2f size{ shape.GetSize() };
			const BinaryEngine::ShapeSpecification outline{ .Color{ color }, .OutlineThickness{ s_QueryLineThickness } };

			if (shape.GetType() == BinaryEngine::QueryShapeType::Capsule)
			{
				DrawCapsule(renderer, center, size, shape.GetRotation(), s_QueryZ, outline);
				return;
			}

			const BinaryEngine::Transform transform{ { center.x, center.y, s_QueryZ }, { 0.0f, 0.0f, shape.GetRotation() }, { size.x, size.y, 1.0f } };
			if (shape.GetType() == BinaryEngine::QueryShapeType::Box)
			{
				renderer.DrawRectangle(transform, outline);
				return;
			}

			renderer.DrawCircle(transform, outline);
		}

	}

	QueriesDemoState::QueriesDemoState(BinaryEngine::StateManager& stateManager, const BinaryEngine::Context& context) :
		State(stateManager, context)
	{
		APP_TRACE("[QueriesDemoState] Created");
	}

	QueriesDemoState::~QueriesDemoState()
	{
		APP_TRACE("[QueriesDemoState] Destroyed");
	}

	void QueriesDemoState::OnAttach()
	{
		APP_INFO("[QueriesDemoState] Attached");

		m_Context.renderer.SetClearColor(s_ClearColor);
		m_AtlasHandle = m_Context.assetManager.LoadAsset<BinaryEngine::Texture2D>(s_PhysicsAtlasPath, m_Context.renderer);
		m_FontHandle = m_Context.assetManager.LoadAsset<BinaryEngine::Font>(s_FontPath, m_Context.renderer);
		m_EyePosition = s_EyeStart;

		const BinaryEngine::TextSpecification hudSpecification{
			.Size{ s_HudTextSize },
			.FillColor{ BinaryEngine::Color::White },
			.Space{ BinaryEngine::RenderSpace::Screen },
			.Alignment{ BinaryEngine::TextAlignment::Left },
		};
		m_HudEntity = m_Scene.CreateEntity("QueriesHud");
		m_HudEntity.AddComponent<BinaryEngine::TextComponent>(m_FontHandle, std::string{}, hudSpecification);
		m_HudEntity.GetComponent<BinaryEngine::TransformComponent>().transform.Position = { 8.0f, 20.0f, s_HudZ };

		CreateLevel();
	}

	void QueriesDemoState::OnDetach()
	{
		APP_INFO("[QueriesDemoState] Detached");
		APP_INFO("[QueriesDemoState] {} ray casts, {} ray-all casts, {} shape casts, {} overlaps, {} point picks", m_RayCastCount, m_RayAllCastCount, m_ShapeCastCount, m_OverlapCount, m_PointPickCount);
	}

	void QueriesDemoState::OnEvent(BinaryEngine::Event& event)
	{
		BinaryEngine::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<BinaryEngine::KeyPressedEvent>(BIND_FUNCTION(OnKeyPressed));
		dispatcher.Dispatch<BinaryEngine::MouseButtonPressedEvent>(BIND_FUNCTION(OnMouseButtonPressed));
	}

	void QueriesDemoState::OnFixedUpdate(BinaryEngine::TimeStep fixedStep)
	{
		m_Scene.OnFixedUpdate(fixedStep);
	}

	void QueriesDemoState::OnUpdate(BinaryEngine::TimeStep dt)
	{
		m_Scene.OnUpdate(dt, m_Context.clock.GetInterpolationAlpha());
	}

	void QueriesDemoState::OnRender()
	{
		BinaryEngine::Renderer& renderer{ m_Context.renderer };
		renderer.BeginScene(m_Camera);
		const std::string queryResult{ RunQuery(renderer) };
		UpdateHud(queryResult);
		m_Scene.OnRender(renderer, m_Context.assetManager);
		DrawCapsuleBodies(renderer);
		renderer.DrawRectangle(BinaryEngine::Transform{ { s_TriggerZoneCenter.x, s_TriggerZoneCenter.y, s_TriggerZoneZ }, { 0.0f, 0.0f, 0.0f }, { s_TriggerZoneSize.x, s_TriggerZoneSize.y, 1.0f } }, { .Color{ s_TriggerZoneColor } });
		if (m_ShowOutlines)
		{
			DrawColliderOutlines(m_Scene, renderer);
		}
		if (m_PickedEntity.IsValid())
		{
			DrawHighlight(renderer, m_PickedEntity);
		}
		renderer.EndScene();
	}

	bool QueriesDemoState::OnKeyPressed(BinaryEngine::KeyPressedEvent& event)
	{
		if (event.IsRepeat())
		{
			return false;
		}

		switch (event.GetKeyCode())
		{
			case BinaryEngine::Key::Num1:
			{
				m_Mode = QueryMode::Ray;
				return true;
			}

			case BinaryEngine::Key::Num2:
			{
				m_Mode = QueryMode::RayAll;
				return true;
			}

			case BinaryEngine::Key::Num3:
			{
				m_Mode = QueryMode::ShapeCast;
				return true;
			}

			case BinaryEngine::Key::Num4:
			{
				m_Mode = QueryMode::Overlap;
				return true;
			}

			case BinaryEngine::Key::S:
			{
				m_ShapeChoice = GetNextShapeChoice(m_ShapeChoice);
				return true;
			}

			case BinaryEngine::Key::R:
			{
				m_ShapeRotationDegrees = std::fmod(m_ShapeRotationDegrees + s_RotationStepDegrees, 360.0f);
				return true;
			}

			case BinaryEngine::Key::E:
			{
				m_EyePosition = GetCursorWorldPosition();
				return true;
			}

			case BinaryEngine::Key::T:
			{
				m_IncludeTriggers = !m_IncludeTriggers;
				return true;
			}

			case BinaryEngine::Key::X:
			{
				RemoveSpawnedBodies();
				return true;
			}

			case BinaryEngine::Key::C:
			{
				m_ShowOutlines = !m_ShowOutlines;
				return true;
			}

			default: return false;
		}
	}

	bool QueriesDemoState::OnMouseButtonPressed(BinaryEngine::MouseButtonPressedEvent& event)
	{
		const BinaryEngine::Vector2f position{ m_Camera.ScreenToWorld(event.GetPosition()) };

		if (event.GetButton() == BinaryEngine::Mouse::Left)
		{
			PickBody(position);
			return true;
		}

		if (event.GetButton() == BinaryEngine::Mouse::Right)
		{
			SpawnCapsule(position);
			return true;
		}

		return false;
	}

	BinaryEngine::Vector2f QueriesDemoState::GetCursorWorldPosition() const
	{
		return m_Camera.ScreenToWorld(BinaryEngine::Input::GetMousePosition());
	}

	BinaryEngine::QueryFilter QueriesDemoState::MakeQueryFilter() const
	{
		return BinaryEngine::QueryFilter{ .IncludeTriggers{ m_IncludeTriggers } };
	}

	BinaryEngine::QueryShape QueriesDemoState::MakeQueryShape(BinaryEngine::Vector2f center) const
	{
		switch (m_ShapeChoice)
		{
			case QueryShapeChoice::Circle: return BinaryEngine::QueryShape::Circle(center, s_QueryCircleRadius);
			case QueryShapeChoice::Box: return BinaryEngine::QueryShape::Box(center, s_QueryBoxSize, m_ShapeRotationDegrees);
			case QueryShapeChoice::Capsule: return BinaryEngine::QueryShape::Capsule(center, s_CapsuleSize, m_ShapeRotationDegrees);
		}

		return BinaryEngine::QueryShape::Circle(center, s_QueryCircleRadius);
	}

	void QueriesDemoState::CreateLevel()
	{
		const BinaryEngine::RigidBodySpecification staticBody{ .Type{ BinaryEngine::BodyType::Static } };
		const BinaryEngine::ColliderSpecification slopeCollider{ .Material{ .Friction{ s_SlopeFriction } } };

		CreateBoxBody("Ground", s_GroundCenter, s_GroundSize, 0.0f, AtlasSquare::Scenery, staticBody, {});
		CreateBoxBody("Ceiling", s_CeilingCenter, s_GroundSize, 0.0f, AtlasSquare::Scenery, staticBody, {});
		CreateBoxBody("LeftWall", s_LeftWallCenter, s_WallSize, 0.0f, AtlasSquare::Scenery, staticBody, {});
		CreateBoxBody("RightWall", s_RightWallCenter, s_WallSize, 0.0f, AtlasSquare::Scenery, staticBody, {});
		CreateBoxBody("Slope", s_SlopeCenter, s_SlopeSize, s_SlopeRotationDegrees, AtlasSquare::Scenery, staticBody, slopeCollider);
		CreateBoxBody("Platform", s_PlatformCenter, s_PlatformSize, 0.0f, AtlasSquare::Scenery, staticBody, {});

		BinaryEngine::Entity triggerZone{ m_Scene.CreateEntity("TriggerZone") };
		triggerZone.GetComponent<BinaryEngine::TransformComponent>().transform.Position = { s_TriggerZoneCenter.x, s_TriggerZoneCenter.y, s_BodyZ };
		triggerZone.AddComponent<BinaryEngine::RigidBodyComponent>(staticBody);
		triggerZone.AddComponent<BinaryEngine::BoxColliderComponent>(BinaryEngine::BoxColliderSpecification{ .Size{ s_TriggerZoneSize }, .Collider{ .IsTrigger{ true } } });

		for (const BinaryEngine::Vector2f& position : s_CratePositions)
		{
			CreateBoxBody("Crate", position, { s_CrateSize, s_CrateSize }, 0.0f, AtlasSquare::Crate, {}, {});
		}

		for (const BinaryEngine::Vector2f& position : s_BallPositions)
		{
			CreateBall(position);
		}

		CreateCapsuleBody("Log", s_LogCenter, s_LogSize, {});
		CreateCapsuleBody("StandingCapsule", s_StandingCapsuleCenter, s_CapsuleSize, BinaryEngine::RigidBodySpecification{ .FixedRotation{ true } });
	}

	void QueriesDemoState::CreateBoxBody(std::string_view name, BinaryEngine::Vector2f position, BinaryEngine::Vector2f size, float rotationDegrees, AtlasSquare square, const BinaryEngine::RigidBodySpecification& body, const BinaryEngine::ColliderSpecification& collider)
	{
		BinaryEngine::Entity entity{ m_Scene.CreateEntity(std::string{ name }) };
		BinaryEngine::Transform& transform{ entity.GetComponent<BinaryEngine::TransformComponent>().transform };
		transform.Position = { position.x, position.y, s_BodyZ };
		transform.Rotation.z = rotationDegrees;
		transform.Scale = GetBoxSpriteScale(size);

		entity.AddComponent<BinaryEngine::SpriteComponent>(m_AtlasHandle, GetAtlasRegion(square));
		entity.AddComponent<BinaryEngine::RigidBodyComponent>(body);
		entity.AddComponent<BinaryEngine::BoxColliderComponent>(BinaryEngine::BoxColliderSpecification{ .Size{ size }, .Collider{ collider } });
	}

	void QueriesDemoState::CreateBall(BinaryEngine::Vector2f position)
	{
		BinaryEngine::Entity entity{ m_Scene.CreateEntity("Ball") };
		BinaryEngine::Transform& transform{ entity.GetComponent<BinaryEngine::TransformComponent>().transform };
		transform.Position = { position.x, position.y, s_BodyZ };
		transform.Scale = GetCircleSpriteScale(s_BallRadius);

		entity.AddComponent<BinaryEngine::SpriteComponent>(m_AtlasHandle, GetAtlasRegion(AtlasDisc::Ball));
		entity.AddComponent<BinaryEngine::RigidBodyComponent>();
		entity.AddComponent<BinaryEngine::CircleColliderComponent>(BinaryEngine::CircleColliderSpecification{ .Radius{ s_BallRadius } });
	}

	BinaryEngine::Entity QueriesDemoState::CreateCapsuleBody(std::string_view name, BinaryEngine::Vector2f position, BinaryEngine::Vector2f size, const BinaryEngine::RigidBodySpecification& body)
	{
		BinaryEngine::Entity entity{ m_Scene.CreateEntity(std::string{ name }) };
		entity.GetComponent<BinaryEngine::TransformComponent>().transform.Position = { position.x, position.y, s_BodyZ };
		entity.AddComponent<BinaryEngine::RigidBodyComponent>(body);
		entity.AddComponent<BinaryEngine::CapsuleColliderComponent>(BinaryEngine::CapsuleColliderSpecification{ .Size{ size } });
		return entity;
	}

	void QueriesDemoState::SpawnCapsule(BinaryEngine::Vector2f position)
	{
		m_SpawnedEntities.push_back(CreateCapsuleBody("Capsule", position, s_CapsuleSize, {}));
	}

	void QueriesDemoState::RemoveSpawnedBodies()
	{
		for (const BinaryEngine::Entity& entity : m_SpawnedEntities)
		{
			if (entity.IsValid())
			{
				m_Scene.DestroyEntity(entity);
			}
		}

		m_SpawnedEntities.clear();
	}

	void QueriesDemoState::PickBody(BinaryEngine::Vector2f position)
	{
		m_PointPickCount++;
		m_PickedEntity = BinaryEngine::Entity{};
		m_Scene.GetPhysicsWorld().OverlapShape(BinaryEngine::QueryShape::Point(position), m_OverlapHits);

		for (const BinaryEngine::OverlapHit& hit : m_OverlapHits)
		{
			BinaryEngine::Entity entity{ hit.Entity };
			if (entity.GetComponent<BinaryEngine::RigidBodyComponent>().Specification.Type == BinaryEngine::BodyType::Dynamic)
			{
				m_PickedEntity = entity;
				return;
			}
		}
	}

	std::string QueriesDemoState::RunQuery(BinaryEngine::Renderer& renderer)
	{
		const BinaryEngine::Vector2f cursor{ GetCursorWorldPosition() };
		renderer.DrawCircle(BinaryEngine::Transform{ { m_EyePosition.x, m_EyePosition.y, s_QueryZ }, { 0.0f, 0.0f, 0.0f }, { s_EyeSize, s_EyeSize, 1.0f } }, { .Color{ s_EyeColor } });

		switch (m_Mode)
		{
			case QueryMode::Ray: return RunRayQuery(renderer, cursor);
			case QueryMode::RayAll: return RunRayAllQuery(renderer, cursor);
			case QueryMode::ShapeCast: return RunShapeCastQuery(renderer, cursor);
			case QueryMode::Overlap: return RunOverlapQuery(renderer, cursor);
		}

		return std::string{};
	}

	std::string QueriesDemoState::RunRayQuery(BinaryEngine::Renderer& renderer, BinaryEngine::Vector2f cursor)
	{
		m_RayCastCount++;
		const std::optional<BinaryEngine::CastHit> hit{ m_Scene.GetPhysicsWorld().CastRay(m_EyePosition, cursor, MakeQueryFilter()) };
		const BinaryEngine::Vector2f lineEnd{ hit ? hit->Point : cursor };
		renderer.DrawLine(m_EyePosition, lineEnd, { .Color{ s_RayColor }, .Thickness{ s_QueryLineThickness }, .SortZ{ s_QueryZ } });
		if (hit)
		{
			DrawCastHit(renderer, *hit);
		}

		return DescribeCastHit(hit);
	}

	std::string QueriesDemoState::RunRayAllQuery(BinaryEngine::Renderer& renderer, BinaryEngine::Vector2f cursor)
	{
		m_RayAllCastCount++;
		const std::size_t hitCount{ m_Scene.GetPhysicsWorld().CastRayAll(m_EyePosition, cursor, m_RayHits, MakeQueryFilter()) };
		renderer.DrawLine(m_EyePosition, cursor, { .Color{ s_RayColor }, .Thickness{ s_QueryLineThickness }, .SortZ{ s_QueryZ } });

		const std::shared_ptr<BinaryEngine::Font> font{ m_Context.assetManager.GetAsset<BinaryEngine::Font>(m_FontHandle) };
		for (std::size_t hitIndex{ 0 }; hitIndex < hitCount; hitIndex++)
		{
			const BinaryEngine::Vector2f point{ m_RayHits[hitIndex].Point };
			renderer.DrawCircle(BinaryEngine::Transform{ { point.x, point.y, s_QueryZ }, { 0.0f, 0.0f, 0.0f }, { s_HitDotSize, s_HitDotSize, 1.0f } }, { .Color{ s_HitColor } });
			if (font)
			{
				const BinaryEngine::Transform labelTransform{ { point.x, point.y + s_MarkerLabelOffset, s_QueryZ }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f } };
				renderer.DrawText(*font, std::to_string(hitIndex + 1), labelTransform, { .Size{ s_MarkerTextSize }, .FillColor{ BinaryEngine::Color::White }, .Alignment{ BinaryEngine::TextAlignment::Center } });
			}
		}

		return std::format("Hits {}", hitCount);
	}

	std::string QueriesDemoState::RunShapeCastQuery(BinaryEngine::Renderer& renderer, BinaryEngine::Vector2f cursor)
	{
		m_ShapeCastCount++;
		const BinaryEngine::QueryShape shape{ MakeQueryShape(m_EyePosition) };
		const BinaryEngine::Vector2f translation{ cursor - m_EyePosition };
		const std::optional<BinaryEngine::CastHit> hit{ m_Scene.GetPhysicsWorld().CastShape(shape, translation, MakeQueryFilter()) };
		const BinaryEngine::Vector2f stopCenter{ hit ? m_EyePosition + translation * hit->Fraction : cursor };

		DrawQueryShape(renderer, shape, s_RayColor);
		renderer.DrawLine(m_EyePosition, stopCenter, { .Color{ s_RayColor }, .Thickness{ s_QueryLineThickness }, .SortZ{ s_QueryZ } });
		DrawQueryShape(renderer, MakeQueryShape(stopCenter), s_GhostColor);
		if (hit)
		{
			DrawCastHit(renderer, *hit);
		}

		return DescribeCastHit(hit);
	}

	std::string QueriesDemoState::RunOverlapQuery(BinaryEngine::Renderer& renderer, BinaryEngine::Vector2f cursor)
	{
		m_OverlapCount++;
		const BinaryEngine::QueryShape shape{ MakeQueryShape(cursor) };
		const std::size_t hitCount{ m_Scene.GetPhysicsWorld().OverlapShape(shape, m_OverlapHits, MakeQueryFilter()) };

		DrawQueryShape(renderer, shape, s_RayColor);
		for (const BinaryEngine::OverlapHit& hit : m_OverlapHits)
		{
			DrawHighlight(renderer, hit.Entity);
		}

		return std::format("Touching {}", hitCount);
	}

	void QueriesDemoState::DrawCapsuleBodies(BinaryEngine::Renderer& renderer)
	{
		m_Scene.GetAllEntitiesWith<BinaryEngine::TransformComponent, BinaryEngine::CapsuleColliderComponent>().each([&renderer](const BinaryEngine::TransformComponent& transformComponent, const BinaryEngine::CapsuleColliderComponent& collider) {
			const BinaryEngine::Transform& transform{ transformComponent.transform };
			DrawCapsule(renderer, { transform.Position.x, transform.Position.y }, collider.Specification.Size, transform.Rotation.z, transform.Position.z, { .Color{ s_CapsuleColor } });
		});
	}

	void QueriesDemoState::DrawHighlight(BinaryEngine::Renderer& renderer, BinaryEngine::Entity entity)
	{
		const BinaryEngine::PhysicsBody body{ m_Scene.GetPhysicsBody(entity) };
		if (!body.IsValid())
		{
			return;
		}

		const BinaryEngine::Vector2f center{ body.GetPosition() };
		const float rotation{ body.GetRotation() };
		const BinaryEngine::ShapeSpecification highlight{ .Color{ s_HighlightColor }, .OutlineThickness{ s_HighlightThickness } };

		if (entity.HasComponent<BinaryEngine::BoxColliderComponent>())
		{
			const BinaryEngine::Vector2f size{ entity.GetComponent<BinaryEngine::BoxColliderComponent>().Specification.Size };
			renderer.DrawRectangle(BinaryEngine::Transform{ { center.x, center.y, s_HighlightZ }, { 0.0f, 0.0f, rotation }, { size.x, size.y, 1.0f } }, highlight);
		}

		if (entity.HasComponent<BinaryEngine::CircleColliderComponent>())
		{
			const float diameter{ entity.GetComponent<BinaryEngine::CircleColliderComponent>().Specification.Radius * 2.0f };
			renderer.DrawCircle(BinaryEngine::Transform{ { center.x, center.y, s_HighlightZ }, { 0.0f, 0.0f, 0.0f }, { diameter, diameter, 1.0f } }, highlight);
		}

		if (entity.HasComponent<BinaryEngine::CapsuleColliderComponent>())
		{
			DrawCapsule(renderer, center, entity.GetComponent<BinaryEngine::CapsuleColliderComponent>().Specification.Size, rotation, s_HighlightZ, highlight);
		}
	}

	void QueriesDemoState::UpdateHud(std::string_view queryResult)
	{
		std::string text{ std::format("{}: {}\n", s_QueryKeyHelp, m_IncludeTriggers ? "on" : "off") };
		text += std::format("{}: {}\n", s_KeyHelp, m_ShowOutlines ? "on" : "off");
		text += std::format("Mode {}   Shape {}, rotated {:.0f}\n", GetModeName(m_Mode), GetShapeName(m_ShapeChoice), m_ShapeRotationDegrees);
		text += std::format("{}\n", queryResult);

		BinaryEngine::Entity pickedEntity{ m_PickedEntity };
		if (pickedEntity.IsValid())
		{
			text += std::format("Bodies {}   Picked '{}' {:.2f} kg", m_Scene.GetPhysicsWorld().GetBodyCount(), GetTag(pickedEntity), m_Scene.GetPhysicsBody(pickedEntity).GetMass());
		}
		else
		{
			text += std::format("Bodies {}   Picked nothing", m_Scene.GetPhysicsWorld().GetBodyCount());
		}

		m_HudEntity.GetComponent<BinaryEngine::TextComponent>().Text = std::move(text);
	}

}
