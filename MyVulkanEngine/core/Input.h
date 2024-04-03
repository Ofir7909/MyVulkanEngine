#pragma once

#include "KeyCode.h"

namespace MVE
{
class Input
{
  public:
	Input() = delete;
	~Input() {}

  public:
	static bool GetKey(KeyCode::KeyCode key);
	// static bool GetKeyDown(KeyCode::KeyCode key);
	// static bool GetKeyUp(KeyCode::KeyCode key);

	static float GetAxis(KeyCode::KeyCode negative, KeyCode::KeyCode positive);
	static glm::vec2 GetVector(KeyCode::KeyCode negativeX, KeyCode::KeyCode positiveX, KeyCode::KeyCode negativeY,
							   KeyCode::KeyCode positiveY);

	static bool GetMouseButton(int button);
	static glm::vec2 GetMousePosition();
};

} // namespace MVE