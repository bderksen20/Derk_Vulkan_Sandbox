/* Application Implementation
	- notes...
*/

#include "app_ctrl.hpp"
#include <stdexcept>
#include <array>
#include <iostream>
namespace vke {

	// Constructor Imp.
	VkeApplication::VkeApplication() {
		loadModels();
		createPipelineLayout();
		createPipeline();
		createCommandBuffers();
	}

	// Destructor Imp.
	VkeApplication::~VkeApplication() {
		vkDestroyPipelineLayout(vkDerkDevice.device(), pipelineLayout, nullptr);
		createPipeline();
		createCommandBuffers();
	}

	void VkeApplication::vke_app_run() {

		// While 
		while (!vkeWindow.shouldClose()) {
			glfwPollEvents();
			drawFrame();
		}

		vkDeviceWaitIdle(vkDerkDevice.device());
	}

	void VkeApplication::loadModels() {
		std::vector<VkeModel::Vertex> vertices{
			{{0.0f, -0.5f}},
			{{0.5f, 0.5f}},
			{{-0.5f, 0.5f}}
		};

		vkeModel = std::make_unique<VkeModel>(vkDerkDevice, vertices);
	}

	void VkeApplication::createPipelineLayout() {

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(vkDerkDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void VkeApplication::createPipeline() {

		PipelineConfigInfo pipelineConfig{};
		VkePipeline::defaultPipelineConfigInfo(pipelineConfig, vkeSwapChain.width(), vkeSwapChain.height());
		pipelineConfig.renderPass = vkeSwapChain.getRenderPass();
		pipelineConfig.pipelineLayout = pipelineLayout;
		vkePipeline = std::make_unique<VkePipeline>(
			vkDerkDevice,
			"simple_shader.vert.spv",
			"simple_shader.frag.spv",
			pipelineConfig);
	}

	// Command buffs recorded and submitted so they can be reused.
	void VkeApplication::createCommandBuffers() {

		commandBuffers.resize(vkeSwapChain.imageCount());	// for now, 1:1 commmand to frame buffer ratio

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;		// primary vs. secondary buffers
		allocInfo.commandPool = vkDerkDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(vkDerkDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers");
		}

		// Record draw commands to buffers
		for (int i = 0; i < commandBuffers.size(); i++) {
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			//std::cout << "beginning command buffer: " << i << '\n';
			if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
				throw std::runtime_error("failed to begin recording command buffer!");
			}

			// First command: begin render pass
			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = vkeSwapChain.getRenderPass();
			renderPassInfo.framebuffer = vkeSwapChain.getFrameBuffer(i);

			// Setup render area
			renderPassInfo.renderArea.offset = { 0,0 };
			renderPassInfo.renderArea.extent = vkeSwapChain.getSwapChainExtent();	//make sure to use swap and not window exten

			// Clear values (what vals we want frame buff to be initially cleared to)
			// structured in a way that: 0 = color attatchment & 1 = depth attatchment
			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
			clearValues[1].depthStencil = { 1.0f, 0 };
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			// Begin render pass
			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);	//inline says that subsequent render commands are part of primary buffer (no secondary used)

			// Bind pipeline & issue command
			vkePipeline->bind(commandBuffers[i]);
			//vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);	// draw command: draw 3 vertices in only 1 instance
			vkeModel->bind(commandBuffers[i]);
			vkeModel->draw(commandBuffers[i]);

			// End render pass
			vkCmdEndRenderPass(commandBuffers[i]);
			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to record command buffer!");
			}

		}
	}

	void VkeApplication::drawFrame() {
		uint32_t imageIndex;
		auto result = vkeSwapChain.acquireNextImage(&imageIndex);	// fetches index of the frame we should render to next (handles cpu+gpu sync)

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		result = vkeSwapChain.submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);	// submits provided command buffer TO graphics queue --> command buff then executed
		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

	}
}