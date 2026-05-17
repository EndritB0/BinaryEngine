#include "pch.h"
#include "BinaryEngine/State/StateManager.h"

namespace BinaryEngine {
	StateManager::StateManager(const Context& context)
		: m_Context(context)
	{
		m_States.reserve(5);
		m_PendingChanges.reserve(5);

		CORE_INFO("[State Manager] Initialized");
	}

	StateManager::~StateManager()
	{
		CleanUp();
		CORE_INFO("[State Manager] Shutdown");
	}

	bool StateManager::HasState()
	{
		return !m_States.empty();
	}

	void StateManager::RequestPopState()
	{
		m_PendingChanges.emplace_back(Action::Pop, nullptr);
	}

	void StateManager::RequestRemoveState(State& target)
	{
		m_PendingChanges.emplace_back(Action::Remove, nullptr, &target);
	}

	void StateManager::RequestClearStates()
	{
		m_PendingChanges.emplace_back(Action::Clear, nullptr);
	}

	void StateManager::ProcessEvent(Event& event)
	{
		for (auto& state : std::views::reverse(m_States))
		{
			state->OnEvent(event);
			if (event.handled)
			{
				break;
			}
		}
	}

	void StateManager::ProcessUpdate(TimeStep dt)
	{
		for (auto& state : m_States)
		{
			state->OnUpdate(dt);
		}
	}

	void StateManager::ProcessRender()
	{
		for (auto& state : m_States)
		{
			state->OnRender();
		}
	}

	void StateManager::ApplyPendingChanges()
	{
		if (m_PendingChanges.empty()) { return; }

		for (auto& change : m_PendingChanges)
		{
			switch (change.action)
			{
				case Action::Push:
				{
					m_States.push_back(std::move(change.state));
					m_States.back()->OnAttach();
					break;
				}

				case Action::Pop:
				{
					if (!m_States.empty())
					{
						m_States.back()->OnDetach();
						m_States.pop_back();
					}
					break;
				}

				case Action::Replace:
				{
					auto targetState{ std::find_if(
						m_States.begin(),
						m_States.end(),
						[&](const std::unique_ptr<State>& state) { return state.get() == change.target; }
					) };

					if (targetState != m_States.end())
					{
						(*targetState)->OnDetach();
						*targetState = std::move(change.state);
						(*targetState)->OnAttach();
						break;
					}
					else
					{
						m_States.push_back(std::move(change.state));
						m_States.back()->OnAttach();
					}
					break;
				}

				case Action::Remove:
				{
					auto targetState{ std::find_if(
						m_States.begin(),
						m_States.end(),
						[&](const std::unique_ptr<State>& state) { return state.get() == change.target; }
					) };

					if (targetState != m_States.end())
					{
						(*targetState)->OnDetach();
						m_States.erase(targetState);
					}
					break;
				}

				case Action::Clear:
				{
					CleanUp();
					break;
				}

				default:
					break;
			}
		}

		m_PendingChanges.clear();
	}

	void StateManager::CleanUp()
	{
		while (!m_States.empty())
		{
			m_States.back()->OnDetach();
			m_States.pop_back();
		}
	}

}