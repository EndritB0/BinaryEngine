#pragma once

#include "BinaryEngine/State/Context.h"
#include "BinaryEngine/Event/Event.h"
#include "BinaryEngine/Core/Timestep.h"

namespace BinaryEngine {

	class StateManager;

	class State {
	public:
		State(StateManager& stateManager, const Context& context) :
			m_StateManager(stateManager), m_Context(context)
		{
		}

		virtual ~State() = default;
		virtual void OnAttach() {};
		virtual void OnDetach() {};
		virtual void OnEvent(Event& event) = 0;
		virtual void OnUpdate(TimeStep dt) = 0;
		virtual void OnRender() = 0;
	protected:
		StateManager& m_StateManager;
		Context m_Context;
	};

}