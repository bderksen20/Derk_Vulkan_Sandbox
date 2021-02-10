/* Application Header 
	- HAS a vke window!
*/
#pragma once

#include "vke_window.hpp"
#include "vke_pipeline.hpp"
#include "vk_derk_device.hpp"
#include "vke_swap_chain.hpp"
#include "vke_model.hpp"

#include <memory>
#include <vector>

namespace vke {

	class VkeApplication {

		public:
			static constexpr int WIDTH = 800;
			static constexpr int HEIGHT = 600;

			VkeApplication();
			~VkeApplication();

			// delete copy constructors
			VkeApplication(const VkeApplication&) = delete;
			VkeApplication& operator = (const VkeApplication&) = delete;

			//void run() {}; empty implementation
			void vke_app_run();

		private:

			void loadModels();
			void createPipelineLayout();
			void createPipeline();
			void createCommandBuffers();
			void drawFrame();

			// Init this app's window!
			VkeWindow vkeWindow{WIDTH, HEIGHT, "VK Window..."};
			VkDerkDevice vkDerkDevice{ vkeWindow };
			VkeSwapChain vkeSwapChain{ vkDerkDevice, vkeWindow.getExtent() };

			// Init graphics pipeline! Removed for new unique pipeline
			// VkePipeline vkePipeline{vkDerkDevice, "simple_shader.vert.spv", "simple_shader.frag.spv", VkePipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT)};
			
			// Smart pointer!!! Automatically handles mem mgmt.
			std::unique_ptr<VkePipeline> vkePipeline;
			VkPipelineLayout pipelineLayout;
			std::vector<VkCommandBuffer> commandBuffers;
			std::unique_ptr<VkeModel> vkeModel;
	};

}