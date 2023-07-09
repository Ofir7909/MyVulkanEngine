#pragma once

namespace MVE
{
class Timestep
{
  public:
	Timestep(float dt): delta {dt} {}

	operator float() const { return delta; }

  private:
	float delta;
};

} // namespace MVE