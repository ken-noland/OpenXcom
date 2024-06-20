#include "UIScript.h"
#include "LuaProperty.h"

#include "StateScript.h"

#include "../Engine/Game.h"
#include "../Engine/State.h"

#include <list>

namespace OpenXcom
{

namespace Lua
{


UIScript::UIScript(Game& game)
	:
	LuaApi("ui"),
	_game(game),
	_onTestEvent("on_test_event")
{

}

UIScript::~UIScript()
{
}

void UIScript::onRegisterApi(lua_State* luaState, int parentTableIndex)
{
	_onTestEvent.registerApi(luaState, parentTableIndex);

	registerProperty<
		[](Game* game) -> State* { return game->getState(); }
	>(luaState, "current_state", parentTableIndex, getGame());

	//registerContainer<
	//	[](Game* game) -> std::list<std::string> { return std::list<std::string>{"state1", "state2"}; }
	//>(luaState, "states", parentTableIndex, getGame());

}

} // namespace Lua

} // namespace OpenXcom
