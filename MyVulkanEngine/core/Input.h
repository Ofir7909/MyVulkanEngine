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

	static bool GetMouseButton(int button);
	static glm::vec2 GetMousePosition();
};

} // namespace MVE