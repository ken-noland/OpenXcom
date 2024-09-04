#include <gtest/gtest.h>

#include "../../Entity/Engine/ECS.h"

using namespace OpenXcom;

TEST(ECSTest, Update)
{
	// Create a new ECS
	ECS ecs;

	ecs.getRegistry();

	// Update the ECS
	ecs.update();
}

