#include "Model.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace MVE
{

Model::Model(Device& device, const Builder& builder): device(device)
{
	CreateVertexBuffers(builder.vertices);
	CreateIndexBuffers(builder.indices);
}

void Model::Bind(VkCommandBuffer commandBuffer)
{
	VkBuffer buffers[]	   = {vertexBuffer->GetBuffer()};
	VkDeviceSize offsets[] = {0};
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

	if (hasIndexBuffer) {
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
	}
}

void Model::Draw(VkCommandBuffer commandBuffer)
{
	if (hasIndexBuffer) {
		vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
	} else {
		vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
	}
}

std::unique_ptr<Model> Model::CreateModelFromFile(Device& device, const std::string& filepath)
{
	Builder builder {};
	builder.LoadModel(filepath);
	MVE_INFO("Loaded Model from '{}'. Vertex count = {}", filepath, builder.vertices.size());
	return std::make_unique<Model>(device, builder);
}

void Model::CreateVertexBuffers(const std::vector<Vertex>& vertices)
{
	vertexCount = vertices.size();
	MVE_ASSERT(vertexCount >= 3, "Vertex Count must be at least 3");
	uint32_t vertexSize		= sizeof(vertices[0]);
	VkDeviceSize bufferSize = vertexCount * vertexSize;

	Buffer stagingBuffer(device, vertexSize, vertexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
						 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	stagingBuffer.Map();
	stagingBuffer.WriteToBuffer((void*)vertices.data());

	vertexBuffer = std::make_unique<Buffer>(device, vertexSize, vertexCount,
											VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
											VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	device.CopyBuffer(stagingBuffer.GetBuffer(), vertexBuffer->GetBuffer(), bufferSize);
}

void Model::CreateIndexBuffers(const std::vector<uint32_t>& indices)
{
	indexCount	   = indices.size();
	hasIndexBuffer = indexCount > 0;
	if (!hasIndexBuffer)
		return;

	MVE_ASSERT(indexCount >= 3, "Index Count must be at least 3, or empty");
	uint32_t indexSize		= sizeof(indices[0]);
	VkDeviceSize bufferSize = indexSize * indexCount;

	Buffer stagingBuffer(device, indexSize, indexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
						 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	stagingBuffer.Map();
	stagingBuffer.WriteToBuffer((void*)indices.data());

	indexBuffer = std::make_unique<Buffer>(device, indexSize, indexCount,
										   VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
										   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	device.CopyBuffer(stagingBuffer.GetBuffer(), indexBuffer->GetBuffer(), bufferSize);
}

std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindingDescriptions()
{
	std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
	bindingDescriptions[0].binding	 = 0;
	bindingDescriptions[0].stride	 = sizeof(Vertex);
	bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions()
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions {};
	attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
	attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
	attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
	attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});

	return attributeDescriptions;
}

void Model::Builder::LoadModel(const std::string& filepath)
{
	vertices.clear();
	indices.clear();

	Assimp::Importer importer {};
	auto scene = importer.ReadFile(filepath, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs |
												 aiProcess_JoinIdenticalVertices);
	MVE_ASSERT(scene, "Failed to load model from {}", filepath);

	for (int i = 0; i < scene->mNumMeshes; i++) {
		auto mesh = scene->mMeshes[i];

		// Vertex buffer
		vertices.reserve(vertices.size() + mesh->mNumVertices);
		for (int j = 0; j < mesh->mNumVertices; j++) {
			Vertex vertex {};
			vertex.position = {mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z};
			vertex.normal	= {mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z};
			if (mesh->mColors[0])
				vertex.color = {mesh->mColors[0][j].r, mesh->mColors[0][j].g, mesh->mColors[0][j].b};
			if (mesh->mTextureCoords[0])
				vertex.uv = {mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y};

			vertices.push_back(vertex);
		}

		// Index buffer
		indices.reserve(indices.size() + mesh->mNumFaces * 3);
		for (int j = 0; j < mesh->mNumFaces; j++) {
			indices.push_back(mesh->mFaces[j].mIndices[0]);
			indices.push_back(mesh->mFaces[j].mIndices[1]);
			indices.push_back(mesh->mFaces[j].mIndices[2]);
		}
	}
}

} // namespace MVE