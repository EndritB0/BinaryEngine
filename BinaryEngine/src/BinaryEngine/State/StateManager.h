#pragma once

#include <memory>
#include <type_traits>
#include <vector>

#include "BinaryEngine/Event/Event.h"
#include "BinaryEngine/State/Context.h"
#include "BinaryEngine/State/State.h"
#include "BinaryEngine/Core/Timestep.h"

namespace BinaryEngine {

	class StateManager {
	public:
		StateManager(const Context& context);
		~StateManager();

		StateManager(const StateManager&) = delete;
		StateManager& operator=(const StateManager&) = delete;

		template<typename T, typename... Args>
			requires(std::is_base_of_v<State, T>)
		void RequestPushState(Args&&... args)
		{
			m_PendingChanges.emplace_back(
				Action::Push,
				std::make_unique<T>(*this, m_Context, std::forward<Args>(args)...)
			);
		}

		template<typename T, typename... Args>
			requires(std::is_base_of_v<State, T>)
		void RequestReplaceState(State& target, Args&&... args)
		{
			m_PendingChanges.emplace_back(
				Action::Replace,
				std::make_unique<T>(*this, m_Context, std::forward<Args>(args)...),
				&target
			);
		}

		bool HasState();
		void RequestPopState();
		void RequestRemoveState(State& target);
		void RequestClearStates();
		void ProcessEvent(Event& event);
		void ProcessUpdate(TimeStep dt);
		void ProcessRender();
		void ApplyPendingChanges();

	private:
		void CleanUp();

	private:
		enum class Action { None = 0, Push, Pop, Replace, Remove, Clear };

		struct PendingChange {
			Action action{ Action::None };
			std::unique_ptr<State> state{ nullptr };
			State* target{ nullptr };
		};

		std::vector<std::unique_ptr<State>> m_States;
		std::vector<PendingChange> m_PendingChanges;
		Context m_Context;

	};

}