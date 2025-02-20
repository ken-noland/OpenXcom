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

#include "../../../Engine/Engine.h"
#include "../../../Engine/Resource/ResourceSystem.h"
#include "../../../Engine/Graphics/Buffer/Buffer.h"
#include "../../../Engine/Graphics/Buffer/BufferManager.h"

#include <filesystem>

using namespace OpenXcom;

class BufferTest : public ::testing::Test
{
protected:
	static std::unique_ptr<Engine> _engine;

	static std::filesystem::path _dataPath;
	static std::filesystem::path _configPath;
	static std::filesystem::path _userPath;

	static void SetUpTestSuite()
	{
		std::filesystem::path path = TEST_DATA_DIR;
		_dataPath = path / "Data";
		_configPath = path / "Config";
		_userPath = path / "User";

		std::vector<std::string> args = {"-data", _dataPath.string(), "-config", _configPath.string(), "-user", _userPath.string(), "-headless"};
		_engine = std::make_unique<Engine>(args);
	}

	static void TearDownTestSuite()
	{
		_engine.reset();
	}

};

std::unique_ptr<Engine> BufferTest::_engine = nullptr;

std::filesystem::path BufferTest::_dataPath;
std::filesystem::path BufferTest::_configPath;
std::filesystem::path BufferTest::_userPath;

TEST_F(BufferTest, TestHostBufferCreate)
{
	ResourceSystem& resourceSystem = _engine->getEngineContext().getResourceSystem();
	BufferManager& bufferManager = resourceSystem.getBufferManager();

	std::unique_ptr<HostBuffer> hostBuffer = bufferManager.createHostBuffer<uint32_t>({1, 2, 3, 4, 5, 6, 7, 8}, BufferUsage::Storage);
	EXPECT_EQ(hostBuffer->getUsage(), BufferUsage::Storage);
	EXPECT_EQ(hostBuffer->getElementSize(), sizeof(uint32_t));
	EXPECT_EQ(hostBuffer->getCount(), 8);
	EXPECT_EQ(hostBuffer->getSize(), 8 * sizeof(uint32_t));
	EXPECT_EQ(hostBuffer->getAllocatedSize(), 8 * sizeof(uint32_t));
}

TEST_F(BufferTest, TestDeviceBufferCreate)
{
	ResourceSystem& resourceSystem = _engine->getEngineContext().getResourceSystem();
	BufferManager& bufferManager = resourceSystem.getBufferManager();

	std::unique_ptr<HostBuffer> hostBuffer = bufferManager.createHostBuffer<uint32_t>({1, 2, 3, 4, 5, 6, 7, 8}, BufferUsage::Storage);
	EXPECT_EQ(hostBuffer->getUsage(), BufferUsage::Storage);
	EXPECT_EQ(hostBuffer->getElementSize(), sizeof(uint32_t));
	EXPECT_EQ(hostBuffer->getCount(), 8);
	EXPECT_EQ(hostBuffer->getSize(), 8 * sizeof(uint32_t));
	EXPECT_EQ(hostBuffer->getAllocatedSize(), 8 * sizeof(uint32_t));

	std::unique_ptr<DeviceBuffer> deviceBuffer = bufferManager.createDeviceBuffer(*hostBuffer);
	EXPECT_EQ(deviceBuffer->getUsage(), BufferUsage::Storage);
	EXPECT_EQ(deviceBuffer->getElementSize(), sizeof(uint32_t));
	EXPECT_EQ(deviceBuffer->getCount(), 8);
	EXPECT_EQ(deviceBuffer->getSize(), 8 * sizeof(uint32_t));
	EXPECT_EQ(deviceBuffer->getAllocatedSize(), 8 * sizeof(uint32_t));
}

TEST_F(BufferTest, TestHostToDeviceToHostBuffer)
{
	ResourceSystem& resourceSystem = _engine->getEngineContext().getResourceSystem();
	BufferManager& bufferManager = resourceSystem.getBufferManager();

	std::unique_ptr<HostBuffer> hostBuffer1 = bufferManager.createHostBuffer<uint32_t>({1, 2, 3, 4, 5, 6, 7, 8}, BufferUsage::Storage);
	EXPECT_EQ(hostBuffer1->getUsage(), BufferUsage::Storage);
	EXPECT_EQ(hostBuffer1->getElementSize(), sizeof(uint32_t));
	EXPECT_EQ(hostBuffer1->getCount(), 8);
	EXPECT_EQ(hostBuffer1->getSize(), 8 * sizeof(uint32_t));
	EXPECT_EQ(hostBuffer1->getAllocatedSize(), 8 * sizeof(uint32_t));

	std::unique_ptr<DeviceBuffer> deviceBuffer = bufferManager.createDeviceBuffer(*hostBuffer1);
	EXPECT_EQ(deviceBuffer->getUsage(), BufferUsage::Storage);
	EXPECT_EQ(deviceBuffer->getElementSize(), sizeof(uint32_t));
	EXPECT_EQ(deviceBuffer->getCount(), 8);
	EXPECT_EQ(deviceBuffer->getSize(), 8 * sizeof(uint32_t));
	EXPECT_EQ(deviceBuffer->getAllocatedSize(), 8 * sizeof(uint32_t));

	std::unique_ptr<HostBuffer> hostBuffer2 = bufferManager.createHostBuffer(*deviceBuffer);
	EXPECT_EQ(hostBuffer2->getUsage(), BufferUsage::Storage);
	EXPECT_EQ(hostBuffer2->getElementSize(), sizeof(uint32_t));
	EXPECT_EQ(hostBuffer2->getCount(), 8);
	EXPECT_EQ(hostBuffer2->getSize(), 8 * sizeof(uint32_t));
	EXPECT_EQ(hostBuffer2->getAllocatedSize(), 8 * sizeof(uint32_t));

	void* data1 = hostBuffer1->map();
	void* data2 = hostBuffer2->map();

	EXPECT_EQ(memcmp(data1, data2, hostBuffer1->getSize()), 0);

	hostBuffer1->unmap();
	hostBuffer2->unmap();
}
