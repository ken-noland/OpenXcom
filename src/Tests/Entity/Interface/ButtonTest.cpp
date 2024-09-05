#include <entt/entt.hpp>
#include <gtest/gtest.h>

#include "../../../Entity/Engine/ECS.h"
#include "../../../Entity/Interface/Interface.h"
#include "../../../Entity/Interface/Button.h"

using namespace OpenXcom;

// Test fixture for the ButtonSystem
class ButtonSystemTest : public ::testing::Test
{
protected:
	ECS ecs;
	entt::handle windowEntity;
	entt::handle textButtonEntity;

	virtual void SetUp()
	{
		InterfaceFactory& factory = ecs.getFactory<InterfaceFactory>();

		InterfaceFactory::CreateWindowParams windowParams = {
			.name = "TestWindow",
			.x = 0,
			.y = 0,
			.width = 100,
			.height = 100,
			.popup = WindowPopup::POPUP_BOTH};
		windowEntity = factory.createWindow(windowParams);

		InterfaceFactory::CreateTextButtonParams textButtonParams = {
			.name = "TestTextButton",
			.text = "Test",
			.x = 10,
			.y = 10,
			.width = 50,
			.height = 20,
			.parent = windowEntity};
		textButtonEntity = factory.createTextButton(textButtonParams);
	}
};


// Test case: Setting and getting the text button's color
TEST_F(ButtonSystemTest, SetAndGetColor)
{
	ButtonSystem& buttonSystem = ecs.getSystem<ButtonSystem>();

	Uint8 color = 42;
	buttonSystem.setColor(textButtonEntity, color);

	// Verify that the color is set correctly
	EXPECT_EQ(buttonSystem.getColor(textButtonEntity), color);
}

// Test case: Drawing the text button (mocking or checking internal state after draw)
TEST_F(ButtonSystemTest, DrawTextButton)
{
	ButtonSystem& buttonSystem = ecs.getSystem<ButtonSystem>();

	// Call draw and verify if the internal state is correctly handled
	//buttonSystem.draw(textButtonEntity);
}

// Test case: Handling input for the text button (you might mock input actions)
TEST_F(ButtonSystemTest, HandleInput)
{
	ButtonSystem& buttonSystem = ecs.getSystem<ButtonSystem>();

	//Action action;
	//// Assuming Action contains some predefined input or click event
	//action.type = ActionType::CLICK; // Example event type

	//textButtonSystem.handle(textButtonEntity, &action);
}
