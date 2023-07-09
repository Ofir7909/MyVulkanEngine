#pragma once

#include "core/Timestep.h"

namespace MVE
{
class Module
{
  public:
	Module()		  = default;
	virtual ~Module() = default;

	virtual void OnAttach() {}
	virtual void OnDetach() {}
	virtual void OnUpdate(Timestep dt) {}
};
} // namespace MVE