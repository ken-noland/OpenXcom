/*
 * Copyright 2010-2016 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <gtest/gtest.h>

#include "../../../Engine/Resource/ResourceManager.h"

using namespace OpenXcom;

class MockData
{
public:
	std::string name;
};

class MockResourceManager : public ResourceManager<MockData>
{
public:
	MockResourceManager()
	{
	}
	virtual ~MockResourceManager()
	{
	}

	ResourceManager<MockData>::OwningHandle load(const std::string& name)
	{
		std::unique_ptr<MockData> data = std::make_unique<MockData>();
		data->name = name;

		ResourceManager<MockData>::OwningHandle handle = add(std::move(data));
		return handle;
	}
};

TEST(ResourceManagerTest, TestResourceManager)
{
	MockResourceManager manager;
	MockResourceManager::OwningHandle handle = manager.load("test");
	ASSERT_TRUE(manager.exists(handle));

	MockData& data = manager.get(handle);
	ASSERT_EQ(data.name, "test");

	manager.remove(std::move(handle));
	ASSERT_FALSE(manager.exists(handle));
}
