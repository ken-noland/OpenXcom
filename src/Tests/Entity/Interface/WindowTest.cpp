#include <gtest/gtest.h>
#include <entt/entt.hpp>

#include "../../../Entity/Engine/ECS.h"
#include "../../../Entity/Interface/Interface.h"
#include "../../../Entity/Interface/Window.h"

using namespace OpenXcom;

// Test fixture for the WindowSystem
class WindowSystemTest : public ::testing::Test
{
protected:
	ECS ecs;
	entt::handle windowEntity;

	virtual void SetUp()
	{
		InterfaceFactory& factory = ecs.getFactory<InterfaceFactory>();

		InterfaceFactory::CreateWindowParams params = {
			.name = "TestWindow",
			.width = 100,
			.height = 100,
			.x = 0,
			.y = 0,
			.popup = WindowPopup::POPUP_BOTH
		};
		windowEntity = factory.createWindow(params);
	}
};

// Test case: Ensure parameters are correctly propagated to the WindowComponent
TEST_F(WindowSystemTest, WindowComponentParameterPropagation)
{
	// Setup parameters for window creation
	InterfaceFactory::CreateWindowParams params = {
		.name = "PropagatedWindow",
		.width = 200,
		.height = 150,
		.x = 10,
		.y = 20,
		.popup = WindowPopup::POPUP_BOTH
	};

	// Create the window with these parameters
	entt::handle newWindowEntity = ecs.getFactory<InterfaceFactory>().createWindow(params);

	// Retrieve the WindowComponent attached to this entity
	const WindowComponent& windowComponent = newWindowEntity.get<WindowComponent>();

	// Check that the parameters were propagated correctly
	EXPECT_EQ(windowComponent._dx, 0);                          // Assuming _dx is initialized to 0
	EXPECT_EQ(windowComponent._dy, 0);                          // Assuming _dy is initialized to 0
	EXPECT_EQ(windowComponent._popup, WindowPopup::POPUP_BOTH); // Popup should match the passed parameter
	EXPECT_EQ(windowComponent._popupStep, 0.0);                 // Assuming _popupStep is initialized to 0
	EXPECT_FALSE(windowComponent._contrast);                    // Check default initialization
	EXPECT_FALSE(windowComponent._screen);                      // Check default initialization
	EXPECT_FALSE(windowComponent._thinBorder);                  // Check default initialization
	EXPECT_EQ(windowComponent._color, 0);                       // Assuming default color is 0
	EXPECT_EQ(windowComponent._innerColor, 0);                  // Assuming default inner color is 0
	EXPECT_FALSE(windowComponent._mute);                        // Assuming default mute is false
	EXPECT_EQ(windowComponent._state, nullptr);                 // Assuming state is nullptr by default

	// You may also verify any other parameters in WindowComponent that are derived from `params`
}

// Test case: Setting and getting window color
TEST_F(WindowSystemTest, SetAndGetColor)
{
	WindowSystem& windowSystem = ecs.getSystem<WindowSystem>();

	uint8_t color = 42;
	windowSystem.setColor(windowEntity, color);

	// Test if the color was correctly set
	EXPECT_EQ(windowSystem.getColor(windowEntity), color);
}

// Test case: Setting and getting delta values (DX and DY)
TEST_F(WindowSystemTest, SetDXAndDY)
{
	WindowSystem& windowSystem = ecs.getSystem<WindowSystem>();

	int dx = 10;
	int dy = 20;

	windowSystem.setDX(windowEntity, dx);
	windowSystem.setDY(windowEntity, dy);

	WindowComponent& windowComponent = windowEntity.get<WindowComponent>();

	EXPECT_EQ(windowComponent._dx, dx);
	EXPECT_EQ(windowComponent._dy, dy);
}

// Test case: Testing high contrast setting
TEST_F(WindowSystemTest, SetHighContrast)
{
	WindowSystem& windowSystem = ecs.getSystem<WindowSystem>();

	bool contrast = true;
	windowSystem.setHighContrast(windowEntity, contrast);

	WindowComponent& windowComponent = windowEntity.get<WindowComponent>();
	EXPECT_TRUE(windowComponent._contrast);
}

// Test case: Setting thin border
TEST_F(WindowSystemTest, SetThinBorder)
{
	WindowSystem& windowSystem = ecs.getSystem<WindowSystem>();

	windowSystem.setThinBorder(windowEntity);

	WindowComponent& windowComponent = windowEntity.get<WindowComponent>();
	EXPECT_TRUE(windowComponent._thinBorder);
}

// Test case: Setting inner color
TEST_F(WindowSystemTest, SetInnerColor)
{
	WindowSystem& windowSystem = ecs.getSystem<WindowSystem>();

	uint8_t innerColor = 12;
	windowSystem.setInnerColor(windowEntity, innerColor);

	WindowComponent& windowComponent = windowEntity.get<WindowComponent>();
	EXPECT_EQ(windowComponent._innerColor, innerColor);
}

// Test case: Add and play popup sound
TEST_F(WindowSystemTest, AddAndPlayPopupSound)
{
	WindowSystem& windowSystem = ecs.getSystem<WindowSystem>();

	// For now, we are just ensuring the function works without errors
	windowSystem.playSound(windowEntity);
}

// Test case: Updating window progress
TEST_F(WindowSystemTest, UpdateWindowProgress)
{
	WindowSystem& windowSystem = ecs.getSystem<WindowSystem>();

	double progress = 0.5;
	windowSystem.UpdateProgress(windowEntity, progress);

	WindowComponent& windowComponent = windowEntity.get<WindowComponent>();
	EXPECT_EQ(windowComponent._popupStep, progress);
}

// Test case: Finalizing window progress
TEST_F(WindowSystemTest, CompleteWindowProgress)
{
	WindowSystem& windowSystem = ecs.getSystem<WindowSystem>();

	windowSystem.CompleteProgress(windowEntity);

	WindowComponent& windowComponent = windowEntity.get<WindowComponent>();
	EXPECT_EQ(windowComponent._popupStep, 1.0); // Assuming complete progress sets it to 1.0
}
