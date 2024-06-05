//> includes
#include "vk_engine.h"

#include <SDL.h>
#include <SDL_vulkan.h>

#include <vk_initializers.h>
#include <vk_types.h>

#include <chrono>
#include <thread>

FlushEngine* loadedEngine = nullptr;

constexpr bool bUseValidationLayers = false;

FlushEngine& FlushEngine::Get() { return *loadedEngine; }
void FlushEngine::init()
{
    // only one engine initialization is allowed with the application.
    assert(loadedEngine == nullptr);
    loadedEngine = this;

    // We initialize SDL and create a window with it.
    SDL_Init(SDL_INIT_VIDEO);

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);

    _window = SDL_CreateWindow(
        "Vulkan Engine",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        _windowExtent.width,
        _windowExtent.height,
        window_flags);
    
    init_vulkan();
    init_spawnchain();
    init_commands();
    init_sync_structs();

    // everything went fine
    _isInitialized = true;
}

void FlushEngine::init_vulkan()
{
    vkb::InstanceBuilder builder;

    auto inst_ret = builder.set_app_name("Vk_flush")
        .request_validation_layers(true) // Change this to bUseVaildtaionLayers 
        .use_default_debug_messenger()
        .require_api_version(1, 3, 0)
        .build();

    vkb::Instance vkb_inst = inst_ret.value();

    // create the surface
    SDL_Vulkan_CreateSurface(_window, _instance, &_surface);

    // Vulkan 1.2 device features
    VkPhysicalDeviceVulkan12Features features12{};
    features12.bufferDeviceAddress = true;
    features12.descriptorIndexing = true;

	// Vulkan 1.3 device features
	VkPhysicalDeviceVulkan13Features features{};
    features.dynamicRendering = true;
    features.synchronization2 = true;

    // using vkbootstrap to pick a physical device.
    // We want a GPU that can render stuff with Vulkan 1.3 and write SDL surface.
    vkb::PhysicalDeviceSelector selector{ vkb_inst };
    vkb::PhysicalDevice physical_device = selector
        .set_minimum_version(1, 3)
        .set_required_features_13(features)
        .set_required_features_12(features12)
        .select()
        .value();

    vkb::DeviceBuilder device_builder{ physical_device };
    //grab the instance from the builder
    _instance = vkb_inst.instance;
    _debug_messenger = vkb_inst.debug_messenger;

    vkb::Device vkb_device = device_builder.build().value();

	_device = vkb_device.device;
	_chosenGPU = physical_device.physical_device;
    
    // everything went fine
    _isInitialized = true;
}


void FlushEngine::init_spawnchain()
{
	create_swapchain(_windowExtent.width, _windowExtent.height);
}

void FlushEngine::init_commands()
{
	// nothing yet
}

void FlushEngine::init_sync_structs()
{
	// nothing yet
}

void FlushEngine::destroy_swapchain()
{
    vkDestroySwapchainKHR(_device, _swapchain, nullptr);

    for(int i =0; i < _swapchainImageViews.size(); i++)
	{
		vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);
	}
}

void FlushEngine::cleanup()
{
    if (_isInitialized) {

        destroy_swapchain();

        vkDestroySurfaceKHR(_instance, _surface, nullptr);
        vkDestroyDevice(_device, nullptr);

        vkb::destroy_debug_utils_messenger(_instance, _debug_messenger);
        vkDestroyInstance(_instance, nullptr);
        SDL_DestroyWindow(_window);
    }

    // clear engine pointer
    loadedEngine = nullptr;
}

void FlushEngine::draw()
{
    // nothing yet
}

void FlushEngine::run()
{
    SDL_Event e;
    bool bQuit = false;

    // main loop
    while (!bQuit) {
        // Handle events on queue
        while (SDL_PollEvent(&e) != 0) {
            // close the window when user alt-f4s or clicks the X button
            if (e.type == SDL_QUIT)
                bQuit = true;

            if (e.type == SDL_WINDOWEVENT) {
                if (e.window.event == SDL_WINDOWEVENT_MINIMIZED) {
                    stop_rendering = true;
                }
                if (e.window.event == SDL_WINDOWEVENT_RESTORED) {
                    stop_rendering = false;
                }
            }
        }

        // do not draw if we are minimized
        if (stop_rendering) {
            // throttle the speed to avoid the endless spinning
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        draw();
    }
}