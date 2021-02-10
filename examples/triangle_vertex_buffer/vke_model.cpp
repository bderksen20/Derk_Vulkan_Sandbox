#include "vke_model.hpp"

#include <cassert>
namespace vke {

	VkeModel::VkeModel(VkDerkDevice& device, const std::vector<Vertex>& vertices) : vkDerkDevice{ device } {
		createVertexBuffers(vertices);
	}

	VkeModel::~VkeModel() {
		vkDestroyBuffer(vkDerkDevice.device(), vertexBuffer, nullptr);
		vkFreeMemory(vkDerkDevice.device(), vertexBufferMemory, nullptr);	// implement VMA???
	}

	void VkeModel::createVertexBuffers(const std::vector<Vertex>& vertices) {

		vertexCount = static_cast<uint32_t>(vertices.size());
		assert(vertexCount >= 3 && "Vertex count must be at least 3!");		// make sure we have at least 3 vertices to form 1 triangle

		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;		// size of vertex * n vertices = buffer size

		// Use createBuffer from device class
		vkDerkDevice.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,												// says this is a vertex buffer
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,		// makes memory accessible and consitent btwn host+device
			vertexBuffer,
			vertexBufferMemory);
	
		void* data;	
		vkMapMemory(vkDerkDevice.device(), vertexBufferMemory, 0, bufferSize, 0, &data);	// data set to point to beginning of mapped memory
		memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(vkDerkDevice.device(), vertexBufferMemory);							// memcpy copies vertice data into host mapped mem--> coherent bit auto flushes mem to device
	}

	void VkeModel::bind(VkCommandBuffer commandBuffer) {
		VkBuffer buffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);	// records to command buffer to bind one vertex buffer at 0 w offset 0
	}

	void VkeModel::draw(VkCommandBuffer commandBuffer) {
		vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
	}

	// Vertex struct's fxn in model header
	std::vector<VkVertexInputBindingDescription> VkeModel::Vertex::getBindingDescriptions() {
		std::vector< VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescriptions;
	}

	std::vector<VkVertexInputAttributeDescription> VkeModel::Vertex::getAttributeDescriptions() {
		std::vector< VkVertexInputAttributeDescription> attributeDescriptions(1);
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = 0;

		return attributeDescriptions;
	}

}