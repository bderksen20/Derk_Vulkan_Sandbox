#pragma once

#include "vk_derk_device.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>

namespace vke {

	// Model Class purpose: take vertex data created/read in by CPU + allocate memory and copy data over to device CPU
	class VkeModel {

	public:

		struct Vertex {
			glm::vec2 position;
			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
		};
		
		VkeModel(VkDerkDevice &device, const std::vector<Vertex>& vertices);
		~VkeModel();

		// MUST delete copy constructors: because model class manages buffers and memory
		VkeModel(const VkeModel&) = delete;
		VkeModel& operator = (const VkeModel&) = delete;

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);

	private:

		void createVertexBuffers(const std::vector<Vertex>& vertices);

		VkDerkDevice& vkDerkDevice;
		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;		// programmer in control of mem mgmt
		uint32_t vertexCount;
	};


}