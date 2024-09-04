#include <gtest/gtest.h>

#include "../../Engine/TypeErasedPtr.h"

using namespace OpenXcom;

TEST(TypeErasedPtrTest, TestTypeConstructor)
{
	int* ptr = new int(5);
	TypeErasedPtr typeErasedPtr(ptr);
	EXPECT_EQ(5, typeErasedPtr.get<int>());
}

TEST(TypeErasedPtrTest, TestFunctionConstructor)
{
	int* ptr = new int(5);
	TypeErasedPtr typeErasedPtr(ptr, [](void* p) { delete static_cast<int*>(p); });
	EXPECT_EQ(5, typeErasedPtr.get<int>());
}

TEST(TypeErasedPtrTest, TestComplexType)
{
	class TestType
	{
	public:
		TestType(std::function<void()> onCreate, std::function<void()> onDestroy) : _value(10), _onCreate(onCreate), _onDestroy(onDestroy) { _onCreate(); }
		~TestType() { _onDestroy(); }

		int _value;

		std::function<void()> _onCreate;
		std::function<void()> _onDestroy;
	};

	bool onCreateCalled = false;
	bool onDestroyCalled = false;

	// create the type erased pointer within a new scope to ensure it is destroyed before the test ends
	{
		TestType* ptr = new TestType([&]() { onCreateCalled = true; }, [&]() { onDestroyCalled = true; });
		TypeErasedPtr typeErasedPtr(ptr);
		EXPECT_EQ(10, typeErasedPtr.get<TestType>()._value);
	}

	EXPECT_TRUE(onCreateCalled);
	EXPECT_TRUE(onDestroyCalled);
}

TEST(TypeErasedUpdatePtrTest, TestTypeHasUpdate)
{
	struct TestType
	{
		int value = 5;
		void update() { value = 10; }
	};

	TestType* ptr = new TestType();
	TypeErasedUpdatePtr typeErasedPtr(ptr);

	EXPECT_EQ(5, typeErasedPtr.get<TestType>().value);
	EXPECT_TRUE(typeErasedPtr.hasUpdate());
}

TEST(TypeErasedUpdatePtrTest, TestTypeNoUpdate)
{
	struct TestType
	{
		int value = 5;
	};

	TestType* ptr = new TestType();
	TypeErasedUpdatePtr typeErasedPtr(ptr);

	EXPECT_EQ(5, typeErasedPtr.get<TestType>().value);
	EXPECT_FALSE(typeErasedPtr.hasUpdate());
}

TEST(TypeErasedUpdatePtrTest, TestUpdateFunction)
{
	struct TestType
	{
		int value = 5;
		void update() { value = 10; }
	};

	TestType* ptr = new TestType();
	TypeErasedUpdatePtr typeErasedPtr(ptr);

	EXPECT_EQ(5, typeErasedPtr.get<TestType>().value);
	EXPECT_TRUE(typeErasedPtr.hasUpdate());

	typeErasedPtr.update();
	EXPECT_EQ(10, typeErasedPtr.get<TestType>().value);
}

// test copy
TEST(TypeErasedUpdatePtrTest, TestMove)
{
	struct TestType
	{
		int value = 5;
		void update() { value = 10; }
	};

	TestType* ptr = new TestType();
	TypeErasedUpdatePtr typeErasedPtr(ptr);

	EXPECT_EQ(5, typeErasedPtr.get<TestType>().value);
	EXPECT_TRUE(typeErasedPtr.hasUpdate());

	TypeErasedUpdatePtr copy = std::move(typeErasedPtr);

	EXPECT_FALSE(typeErasedPtr.valid());

	EXPECT_EQ(5, copy.get<TestType>().value);
	EXPECT_TRUE(copy.hasUpdate());

	EXPECT_EQ(ptr, &copy.get<TestType>());

	copy.update();

	EXPECT_FALSE(typeErasedPtr.valid());

	EXPECT_EQ(10, copy.get<TestType>().value);
}
