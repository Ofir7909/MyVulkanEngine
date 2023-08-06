#pragma once

#include "Buffer.h"
#include "Device.h"

namespace MVE
{
class Model
{
  public:
	struct Vertex
	{
		glm::vec3 position {};
		glm::vec3 color {1.0f};
		glm::vec3 normal {};
		glm::vec3 tanget {};
		glm::vec3 bitanget {};
		glm::vec2 uv {};

		static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
		static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
	};
	struct Builder
	{
		std::vector<Vertex> vertices {};
		std::vector<uint32_t> indices {};

		void LoadModel(const std::string& filepath);
	};

  public:
	Model(Device& device, const Builder& builder);
	~Model() = default;

	Model(const Model&)			 = delete;
	void operator=(const Model&) = delete;

	void Bind(VkCommandBuffer commandBuffer);
	void Draw(VkCommandBuffer commandBuffer);

  public:
	static std::unique_ptr<Model> CreateModelFromFile(Device& device, const std::string& filepath);

  private:
	void CreateVertexBuffers(const std::vector<Vertex>& vertices);
	void CreateIndexBuffers(const std::vector<uint32_t>& indices);

  private:
	Device& device;

	std::unique_ptr<Buffer> vertexBuffer;
	uint32_t vertexCount;

	bool hasIndexBuffer = false;
	std::unique_ptr<Buffer> indexBuffer;
	uint32_t indexCount;
};
} // namespace MVE
