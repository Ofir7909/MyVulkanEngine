find_package(glfw3 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
#find_package(imgui CONFIG REQUIRED)

set (ENGINE_SRC_FILES
	"main.cpp"
	"core/Application.cpp"
	"core/Window.cpp"
	"moduels/render3d/Pipeline.cpp"
	"moduels/render3d/Render3DModule.cpp"
	"moduels/render3d/Device.cpp"
	"moduels/render3d/SwapChain.cpp"
	"moduels/render3d/Model.cpp"
	"moduels/render3d/Renderer.cpp"
	"moduels/render3d/renderSystems/PbrRenderSystem.cpp"
	"moduels/render3d/Camera.cpp"
	"core/Input.cpp"
	"core/GameObject.cpp"
	"moduels/render3d/Buffer.cpp"
	"moduels/render3d/Descriptors.cpp"
	"moduels/render3d/renderSystems/PointLightSystem.cpp"
	"moduels/render3d/renderSystems/SkyboxSystem.cpp"
	"moduels/render3d/MaterialSystem.cpp"
	"moduels/render3d/Texture.cpp"
	"moduels/render3d/Cubemap.cpp"
)

set (ENGINE_HEADER_FILES
	"core/Log.h"
	"core/Event.h"
	"core/EventCallback.h"
	"moduels/Module.h"
	"moduels/render3d/Render3DModule.h"
	"core/Application.h"
	"core/Window.h"
	"moduels/render3d/Pipeline.h"
	"moduels/render3d/Device.h"
	"moduels/render3d/SwapChain.h"
	"moduels/render3d/Model.h"
	"core/GameObject.h"
	"moduels/render3d/Renderer.h"
	"moduels/render3d/renderSystems/PbrRenderSystem.h"
	"moduels/render3d/Camera.h"
	"core/Timestep.h"
	"core/Input.h"
	"core/KeyCode.h"
	"moduels/render3d/Buffer.h"
	"moduels/render3d/FrameInfo.h"
	"moduels/render3d/Descriptors.h"
	"moduels/render3d/renderSystems/PointLightSystem.h"
	"moduels/render3d/renderSystems/SkyboxSystem.h"
	"moduels/render3d/MaterialSystem.h"
	"moduels/render3d/Texture.h"
	"moduels/render3d/Cubemap.h"
)

add_executable (VulanEngine ${ENGINE_SRC_FILES} ${ENGINE_HEADER_FILES})

set_property(TARGET VulanEngine PROPERTY CXX_STANDARD 20)

target_include_directories(VulanEngine PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

target_precompile_headers(VulanEngine PRIVATE "pch.h")

target_link_libraries(VulanEngine 
	PUBLIC 
		glfw
		#imgui::imgui
		spdlog::spdlog
		glm::glm
		Vulkan::Vulkan
		assimp::assimp
)

# Shaders
if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "AMD64")
  set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin/glslangValidator.exe")
else()
  set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin32/glslangValidator.exe")
endif()

file(GLOB_RECURSE GLSL_SOURCE_FILES
    "shaders/*.frag"
    "shaders/*.vert"
    "shaders/*.comp"
    )

target_compile_definitions(VulanEngine PRIVATE RES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/res/")

target_compile_definitions(VulanEngine PRIVATE SHADER_BINARY_DIR="${PROJECT_BINARY_DIR}/shaders/")

foreach(GLSL ${GLSL_SOURCE_FILES})
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "${PROJECT_BINARY_DIR}/shaders/${FILE_NAME}.spv")
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/shaders/"
    COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
    Shaders 
    DEPENDS ${SPIRV_BINARY_FILES}
    )

add_dependencies(VulanEngine Shaders)

add_custom_command(TARGET VulanEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:VulanEngine>/shaders/"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_BINARY_DIR}/shaders"
        "$<TARGET_FILE_DIR:VulanEngine>/shaders"
        )