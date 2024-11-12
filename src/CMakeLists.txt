# Define option to enable or disable the EntityInspector
option(ENABLE_ENTITY_INSPECTOR "Enable the EntityInspector for debugging and editing entities." ON)

set ( root_src
  "lodepng.cpp"
  "md5.cpp"
)
set ( basescape_src
#  "Basescape/BaseInfoState.cpp"
#  "Basescape/BasescapeState.cpp"
#  "Basescape/BaseView.cpp"
#  "Basescape/BuildFacilitiesState.cpp"
#  "Basescape/CraftArmorState.cpp"
#  "Basescape/CraftEquipmentLoadState.cpp"
#  "Basescape/CraftEquipmentSaveState.cpp"
#  "Basescape/CraftEquipmentState.cpp"
#  "Basescape/CraftInfoState.cpp"
#  "Basescape/CraftPilotSelectState.cpp"
#  "Basescape/CraftPilotsState.cpp"
#  "Basescape/CraftSoldiersState.cpp"
#  "Basescape/CraftsState.cpp"
#  "Basescape/CraftWeaponsState.cpp"
#  "Basescape/DismantleFacilityState.cpp"
#  "Basescape/GlobalAlienContainmentState.cpp"
#  "Basescape/GlobalManufactureState.cpp"
#  "Basescape/GlobalResearchState.cpp"
#  "Basescape/ManageAlienContainmentState.cpp"
#  "Basescape/ManufactureDependenciesTreeState.cpp"
#  "Basescape/ManufactureInfoState.cpp"
#  "Basescape/ManufactureStartState.cpp"
#  "Basescape/ManufactureState.cpp"
#  "Basescape/MiniBaseView.cpp"
#  "Basescape/MonthlyCostsState.cpp"
#  "Basescape/NewManufactureListState.cpp"
#  "Basescape/NewResearchListState.cpp"
#  "Basescape/PlaceFacilityState.cpp"
#  "Basescape/PlaceLiftState.cpp"
#  "Basescape/PlaceStartFacilityState.cpp"
#  "Basescape/PurchaseState.cpp"
#  "Basescape/ResearchInfoState.cpp"
#  "Basescape/ResearchState.cpp"
#  "Basescape/SackSoldierState.cpp"
#  "Basescape/SelectStartFacilityState.cpp"
#  "Basescape/SellState.cpp"
#  "Basescape/SoldierArmorState.cpp"
#  "Basescape/SoldierAvatarState.cpp"
#  "Basescape/SoldierBonusState.cpp"
#  "Basescape/SoldierDiaryMissionState.cpp"
#  "Basescape/SoldierDiaryOverviewState.cpp"
#  "Basescape/SoldierDiaryPerformanceState.cpp"
#  "Basescape/SoldierInfoState.cpp"
#  "Basescape/SoldierMemorialState.cpp"
#  "Basescape/SoldierRankState.cpp"
#  "Basescape/SoldierSortUtil.cpp"
#  "Basescape/SoldiersAIState.cpp"
#  "Basescape/SoldiersState.cpp"
#  "Basescape/SoldierTransformationState.cpp"
#  "Basescape/SoldierTransformationListState.cpp"
#  "Basescape/SoldierTransformState.cpp"
#  "Basescape/StoresState.cpp"
#  "Basescape/TechTreeSelectState.cpp"
#  "Basescape/TechTreeViewerState.cpp"
#  "Basescape/TransferBaseState.cpp"
#  "Basescape/TransferConfirmState.cpp"
#  "Basescape/TransferItemsState.cpp"
#  "Basescape/TransfersState.cpp"
)

set ( battlescape_src
#  "Battlescape/AbortMissionState.cpp"
#  "Battlescape/ActionMenuItem.cpp"
#  "Battlescape/ActionMenuState.cpp"
#  "Battlescape/AIModule.cpp"
#  "Battlescape/AlienInventory.cpp"
#  "Battlescape/AlienInventoryState.cpp"
#  "Battlescape/AliensCrashState.cpp"
#  "Battlescape/BattlescapeGame.cpp"
#  "Battlescape/BattlescapeGenerator.cpp"
#  "Battlescape/BattlescapeMessage.cpp"
#  "Battlescape/BattlescapeState.cpp"
#  "Battlescape/BattleState.cpp"
#  "Battlescape/BriefingLightState.cpp"
#  "Battlescape/BriefingState.cpp"
#  "Battlescape/Camera.cpp"
#  "Battlescape/CannotReequipState.cpp"
#  "Battlescape/CommendationLateState.cpp"
#  "Battlescape/CommendationState.cpp"
#  "Battlescape/ConfirmEndMissionState.cpp"
#  "Battlescape/DebriefingState.cpp"
#  "Battlescape/Explosion.cpp"
#  "Battlescape/ExplosionBState.cpp"
#  "Battlescape/ExtendedBattlescapeLinksState.cpp"
#  "Battlescape/ExtendedInventoryLinksState.cpp"
#  "Battlescape/InfoboxOKState.cpp"
#  "Battlescape/InfoboxState.cpp"
#  "Battlescape/Inventory.cpp"
#  "Battlescape/InventoryLoadState.cpp"
#  "Battlescape/InventoryPersonalState.cpp"
#  "Battlescape/InventorySaveState.cpp"
#  "Battlescape/InventoryState.cpp"
#  "Battlescape/ItemSprite.cpp"
#  "Battlescape/Map.cpp"
#  "Battlescape/MedikitState.cpp"
#  "Battlescape/MedikitView.cpp"
#  "Battlescape/MeleeAttackBState.cpp"
#  "Battlescape/MiniMapState.cpp"
#  "Battlescape/MiniMapView.cpp"
#  "Battlescape/NextTurnState.cpp"
#  "Battlescape/Particle.cpp"
#  "Battlescape/Pathfinding.cpp"
#  "Battlescape/PathfindingNode.cpp"
#  "Battlescape/PathfindingOpenSet.cpp"
#  "Battlescape/PrimeGrenadeState.cpp"
#  "Battlescape/Projectile.cpp"
#  "Battlescape/ProjectileFlyBState.cpp"
#  "Battlescape/PromotionsState.cpp"
#  "Battlescape/PsiAttackBState.cpp"
#  "Battlescape/ScannerState.cpp"
#  "Battlescape/ScannerView.cpp"
#  "Battlescape/SkillMenuState.cpp"
#  "Battlescape/TileEngine.cpp"
#  "Battlescape/TurnDiaryState.cpp"
#  "Battlescape/UnitDieBState.cpp"
#  "Battlescape/UnitFallBState.cpp"
#  "Battlescape/UnitInfoState.cpp"
#  "Battlescape/UnitPanicBState.cpp"
#  "Battlescape/UnitSprite.cpp"
#  "Battlescape/UnitTurnBState.cpp"
#  "Battlescape/UnitWalkBState.cpp"
#  "Battlescape/WarningMessage.cpp"
)

set ( engine_src
#  "Engine/Adlib/adlplayer.cpp"
#  "Engine/Adlib/fmopl.cpp"
  "Engine/Filesystem/CompositeFileSystem.cpp"
  "Engine/Filesystem/EmbeddedFileSystem.cpp"
  "Engine/Filesystem/PhysicalFileSystem.cpp"
  "Engine/Filesystem/VirtualFileSystem.cpp"
  "Engine/Graphics/Vulkan/Font/VulkanFont.cpp"
  "Engine/Graphics/Vulkan/Font/VulkanFontManager.cpp"
  "Engine/Graphics/Vulkan/Palette/VulkanPalette.cpp"
  "Engine/Graphics/Vulkan/Palette/VulkanPaletteManager.cpp"
  "Engine/Graphics/Vulkan/Shader/VulkanShader.cpp"
  "Engine/Graphics/Vulkan/Shader/VulkanShaderManager.cpp"
  "Engine/Graphics/Vulkan/VMAImplementation.cpp"
  "Engine/Graphics/Vulkan/VulkanBuffer.cpp"
  "Engine/Graphics/Vulkan/VulkanImage.cpp"
  "Engine/Graphics/Vulkan/VulkanPipeline.cpp"
  "Engine/Graphics/Vulkan/VulkanResource.cpp"
  "Engine/Graphics/Vulkan/VulkanSurface.cpp"
  "Engine/Graphics/Vulkan/VulkanSystem.cpp"
  "Engine/Graphics/GraphicsSystem.cpp"
  "Engine/Platform/Linux/LinuxProcess.cpp"
  "Engine/Platform/Linux/LinuxWindow.cpp"
  "Engine/Platform/Win32/Win32Process.cpp"
  "Engine/Platform/Win32/Win32Window.cpp"
  "Engine/Platform/ProcessSystem.cpp"
  "Engine/Platform/Window.cpp"
  "Engine/Platform/WindowSystem.cpp"
  "Engine/Resource/ResourceSystem.cpp"
  #"Engine/Resource/Font/Font.cpp"
  "Engine/Resource/Font/FontManager.cpp"
  #"Engine/Resource/Palette/Palette.cpp"
  "Engine/Resource/Palette/PaletteManager.cpp"
  "Engine/Resource/Shader/Shader.cpp"
  "Engine/Resource/Shader/ShaderManager.cpp"
#  "Engine/Action.cpp"
#  "Engine/AdlibMusic.cpp"
#  "Engine/CatFile.cpp"
  "Engine/CrossPlatform.cpp"
  "Engine/Engine.cpp"
  "Engine/Exception.cpp"
#  "Engine/FastLineClip.cpp"
#  "Engine/FileMap.cpp"
#  "Engine/FlcPlayer.cpp"
#  "Engine/Font.cpp"
  "Engine/Game.cpp"
#  "Engine/GMCat.cpp"
#  "Engine/InteractiveSurface.cpp"
  "Engine/Json.cpp"
  "Engine/Language.cpp"
  "Engine/LanguagePlurality.cpp"
  "Engine/LocalizedText.cpp"
  "Engine/Logger.cpp"
#  "Engine/ModInfo.cpp"
#  "Engine/Music.cpp"
#  "Engine/OpenGL.cpp"
#  "Engine/OptionInfo.cpp"
  "Engine/Options.cpp"
#  "Engine/Palette.cpp"
  "Engine/Registry.cpp"
#  "Engine/RNG.cpp"
#  "Engine/Scalers/hq2x.cpp"
#  "Engine/Scalers/hq3x.cpp"
#  "Engine/Scalers/hq4x.cpp"
#  "Engine/Scalers/init.cpp"
#  "Engine/Scalers/scale2x.cpp"
#  "Engine/Scalers/scale3x.cpp"
#  "Engine/Scalers/scalebit.cpp"
#  "Engine/Scalers/xbrz.cpp"
#  "Engine/Screen.cpp"
#  "Engine/Script.cpp"
#  "Engine/Sound.cpp"
#  "Engine/SoundSet.cpp"
   "Engine/State.cpp"
#  "Engine/Surface.cpp"
#  "Engine/SurfaceSet.cpp"
  "Engine/Timer.cpp"
  "Engine/Unicode.cpp"
#  "Engine/Zoom.cpp
)

set ( geoscape_src
#  "Geoscape/AlienBaseState.cpp"
#  "Geoscape/AllocatePsiTrainingState.cpp"
#  "Geoscape/AllocateTrainingState.cpp"
#  "Geoscape/BaseDefenseState.cpp"
#  "Geoscape/BaseDestroyedState.cpp"
#  "Geoscape/BaseNameState.cpp"
#  "Geoscape/BuildNewBaseState.cpp"
#  "Geoscape/ConfirmCydoniaState.cpp"
#  "Geoscape/ConfirmDestinationState.cpp"
#  "Geoscape/ConfirmLandingState.cpp"
#  "Geoscape/ConfirmNewBaseState.cpp"
#  "Geoscape/CraftErrorState.cpp"
#  "Geoscape/CraftNotEnoughPilotsState.cpp"
#  "Geoscape/CraftPatrolState.cpp"
#  "Geoscape/DogfightErrorState.cpp"
#  "Geoscape/DogfightExperienceState.cpp"
#  "Geoscape/DogfightState.cpp"
#  "Geoscape/ExtendedGeoscapeLinksState.cpp"
#  "Geoscape/FundingState.cpp"
#  "Geoscape/GeoscapeCraftState.cpp"
#  "Geoscape/GeoscapeEventState.cpp"
#  "Geoscape/GeoscapeState.cpp"
#  "Geoscape/Globe.cpp"
#  "Geoscape/GraphsState.cpp"
#  "Geoscape/HiddenAlienActivityState.cpp"
#  "Geoscape/InterceptState.cpp"
#  "Geoscape/ItemsArrivingState.cpp"
#  "Geoscape/LowFuelState.cpp"
#  "Geoscape/MissionDetectedState.cpp"
#  "Geoscape/MonthlyReportState.cpp"
#  "Geoscape/MultipleTargetsState.cpp"
#  "Geoscape/NewPossibleCraftState.cpp"
#  "Geoscape/NewPossibleFacilityState.cpp"
#  "Geoscape/NewPossibleManufactureState.cpp"
#  "Geoscape/NewPossiblePurchaseState.cpp"
#  "Geoscape/NewPossibleResearchState.cpp"
#  "Geoscape/ProductionCompleteState.cpp"
#  "Geoscape/PsiTrainingState.cpp"
#  "Geoscape/ResearchCompleteState.cpp"
#  "Geoscape/ResearchRequiredState.cpp"
#  "Geoscape/SelectDestinationState.cpp"
#  "Geoscape/SelectMusicTrackState.cpp"
#  "Geoscape/TargetInfoState.cpp"
#  "Geoscape/TrainingFinishedState.cpp"
#  "Geoscape/TrainingState.cpp"
#  "Geoscape/UfoDetectedState.cpp"
#  "Geoscape/UfoLostState.cpp"
#  "Geoscape/UfoTrackerState.cpp"
)

set ( interface_src
#  "Interface/ArrowButton.cpp"
#  "Interface/Bar.cpp"
#  "Interface/BattlescapeButton.cpp"
#  "Interface/ComboBox.cpp"
#  "Interface/Cursor.cpp"
#  "Interface/FpsCounter.cpp"
#  "Interface/Frame.cpp"
#  "Interface/ImageButton.cpp"
#  "Interface/NumberText.cpp"
#  "Interface/ProgressBar.cpp"
#  "Interface/ScrollBar.cpp"
#  "Interface/Slider.cpp"
#  "Interface/Text.cpp"
#  "Interface/TextButton.cpp"
#  "Interface/TextEdit.cpp"
#  "Interface/TextList.cpp"
#  "Interface/ToggleTextButton.cpp"
#  "Interface/Window.cpp"
)

set ( menu_src
#  "Menu/AbandonGameState.cpp"
#  "Menu/ConfirmLoadState.cpp"
#  "Menu/CutsceneState.cpp"
#  "Menu/DeleteGameState.cpp"
#  "Menu/ErrorMessageState.cpp"
#  "Menu/ListGamesState.cpp"
#  "Menu/ListLoadOriginalState.cpp"
#  "Menu/ListLoadState.cpp"
#  "Menu/ListSaveState.cpp"
#  "Menu/LoadGameState.cpp"
#  "Menu/MainMenuState.cpp"
#  "Menu/NewBattleState.cpp"
#  "Menu/NewGameState.cpp"
#  "Menu/NotesState.cpp"
#  "Menu/OptionsAdvancedState.cpp"
#  "Menu/OptionsAudioState.cpp"
#  "Menu/OptionsBaseState.cpp"
#  "Menu/OptionsBattlescapeState.cpp"
#  "Menu/OptionsConfirmState.cpp"
#  "Menu/OptionsControlsState.cpp"
#  "Menu/OptionsDefaultsState.cpp"
#  "Menu/OptionsFoldersState.cpp"
#  "Menu/OptionsGeoscapeState.cpp"
#  "Menu/ModConfirmExtendedState.cpp"
#  "Menu/ModListState.cpp"
#  "Menu/OptionsNoAudioState.cpp"
#  "Menu/OptionsVideoState.cpp"
#  "Menu/PauseState.cpp"
#  "Menu/SaveGameState.cpp"
#  "Menu/SetWindowedRootState.cpp"
#  "Menu/SlideshowState.cpp"
  "Menu/StartState.cpp"
#  "Menu/StatisticsState.cpp"
#  "Menu/TestPaletteState.cpp"
#  "Menu/TestState.cpp"
#  "Menu/VideoState.cpp"
)

set ( mod_src
#  "Mod/AlienDeployment.cpp"
#  "Mod/AlienRace.cpp"
#  "Mod/Armor.cpp"
#  "Mod/ArticleDefinition.cpp"
#  "Mod/City.cpp"
#  "Mod/CustomPalettes.cpp"
#  "Mod/ExtraSounds.cpp"
#  "Mod/ExtraSprites.cpp"
#  "Mod/ExtraStrings.cpp"
#  "Mod/MapBlock.cpp"
#  "Mod/MapData.cpp"
#  "Mod/MapDataSet.cpp"
#  "Mod/MapScript.cpp"
#  "Mod/MCDPatch.cpp"
#  "Mod/Mod.cpp"
#  "Mod/ModFile.cpp"
#  "Mod/Polygon.cpp"
#  "Mod/Polyline.cpp"
#  "Mod/RuleAlienMission.cpp"
#  "Mod/RuleArcScript.cpp"
#  "Mod/RuleBaseFacility.cpp"
#  "Mod/RuleCommendations.cpp"
#  "Mod/RuleConverter.cpp"
#  "Mod/RuleCountry.cpp"
#  "Mod/RuleCraft.cpp"
#  "Mod/RuleCraftWeapon.cpp"
#  "Mod/RuleDamageType.cpp"
#  "Mod/RuleEnviroEffects.cpp"
#  "Mod/RuleEvent.cpp"
#  "Mod/RuleEventScript.cpp"
#  "Mod/RuleGlobe.cpp"
#  "Mod/RuleInterface.cpp"
#  "Mod/RuleInventory.cpp"
#  "Mod/RuleItem.cpp"
#  "Mod/RuleItemCategory.cpp"
#  "Mod/RuleManufacture.cpp"
#  "Mod/RuleManufactureShortcut.cpp"
#  "Mod/RuleMissionScript.cpp"
#  "Mod/RuleMusic.cpp"
#  "Mod/RuleRegion.cpp"
#  "Mod/RuleResearch.cpp"
#  "Mod/RuleSkill.cpp"
#  "Mod/RuleSoldier.cpp"
#  "Mod/RuleSoldierBonus.cpp"
#  "Mod/RuleSoldierTransformation.cpp"
#  "Mod/RuleStartingCondition.cpp"
#  "Mod/RuleStatBonus.cpp"
#  "Mod/RuleTerrain.cpp"
#  "Mod/RuleUfo.cpp"
#  "Mod/RuleVideo.cpp"
#  "Mod/SoldierNamePool.cpp"
#  "Mod/SoundDefinition.cpp"
#  "Mod/StatString.cpp"
#  "Mod/StatStringCondition.cpp"
#  "Mod/Texture.cpp"
#  "Mod/UfoTrajectory.cpp"
#  "Mod/Unit.cpp
)

set ( savegame_src
#  "Savegame/AlienBase.cpp"
#  "Savegame/AlienMission.cpp"
#  "Savegame/AlienStrategy.cpp"
#  "Savegame/AreaSystem.cpp"
#  "Savegame/Base.cpp"
#  "Savegame/BaseFacility.cpp"
#  "Savegame/BaseSystem.cpp"
#  "Savegame/BattleItem.cpp"
#  "Savegame/BattleUnit.cpp"
#  "Savegame/Country.cpp"
#  "Savegame/CountrySystem.cpp"
#  "Savegame/Craft.cpp"
#  "Savegame/CraftWeapon.cpp"
#  "Savegame/CraftWeaponProjectile.cpp"
#  "Savegame/EquipmentLayoutItem.cpp"
#  "Savegame/GameTime.cpp"
#  "Savegame/GeoscapeEvent.cpp"
#  "Savegame/HitLog.cpp"
#  "Savegame/ItemContainer.cpp"
#  "Savegame/MissionSite.cpp"
#  "Savegame/MovingTarget.cpp"
#  "Savegame/Node.cpp"
#  "Savegame/Production.cpp"
#  "Savegame/RankCount.cpp"
#  "Savegame/Region.cpp"
#  "Savegame/ResearchProject.cpp"
#  "Savegame/SaveConverter.cpp"
#  "Savegame/SavedBattleGame.cpp"
#  "Savegame/SavedGame.cpp"
#  "Savegame/SerializationHelper.cpp"
#  "Savegame/Soldier.cpp"
#  "Savegame/SoldierAvatar.cpp"
#  "Savegame/SoldierDeath.cpp"
#  "Savegame/SoldierDiary.cpp"
#  "Savegame/Target.cpp"
#  "Savegame/Tile.cpp"
#  "Savegame/Transfer.cpp"
#  "Savegame/Ufo.cpp"
#  "Savegame/Vehicle.cpp"
#  "Savegame/Waypoint.cpp"
#  "Savegame/WeightedOptions.cpp
)

set ( ufopedia_src
#  "Ufopaedia/ArticleState.cpp"
#  "Ufopaedia/ArticleStateArmor.cpp"
#  "Ufopaedia/ArticleStateBaseFacility.cpp"
#  "Ufopaedia/ArticleStateCraft.cpp"
#  "Ufopaedia/ArticleStateCraftWeapon.cpp"
#  "Ufopaedia/ArticleStateItem.cpp"
#  "Ufopaedia/ArticleStateText.cpp"
#  "Ufopaedia/ArticleStateTextImage.cpp"
#  "Ufopaedia/ArticleStateTFTD.cpp"
#  "Ufopaedia/ArticleStateTFTDArmor.cpp"
#  "Ufopaedia/ArticleStateTFTDCraft.cpp"
#  "Ufopaedia/ArticleStateTFTDCraftWeapon.cpp"
#  "Ufopaedia/ArticleStateTFTDFacility.cpp"
#  "Ufopaedia/ArticleStateTFTDItem.cpp"
#  "Ufopaedia/ArticleStateTFTDUso.cpp"
#  "Ufopaedia/ArticleStateTFTDVehicle.cpp"
#  "Ufopaedia/ArticleStateUfo.cpp"
#  "Ufopaedia/ArticleStateVehicle.cpp"
#  "Ufopaedia/StatsForNerdsState.cpp"
#  "Ufopaedia/Ufopaedia.cpp"
#  "Ufopaedia/UfopaediaSelectState.cpp"
#  "Ufopaedia/UfopaediaStartState.cpp
)

set ( lua_src
#  "Lua/BasescapeScript.cpp"
#  "Lua/BaseScript.cpp"
#  "Lua/BattlescapeScript.cpp"
#  "Lua/GameScript.cpp"
#  "Lua/GeoscapeScript.cpp"
#  "Lua/LuaApi.cpp"
#  "Lua/LuaArg.cpp"
#  "Lua/LuaDispatch.cpp"
#  "Lua/LuaHelpers.cpp"
#  "Lua/LuaMod.cpp"
#  "Lua/LuaState.cpp"
#  "Lua/SoldierScript.cpp"
#  "Lua/StateScript.cpp"
#  "Lua/UIScript.cpp
)

set ( entity_common_src
#  "Entity/Common/Name.cpp"
)

set ( entity_engine_src
  "Entity/Engine/Drawable.cpp"
  "Entity/Engine/ECS.cpp"
  "Entity/Engine/Hierarchical.cpp"
#  "Entity/Engine/Input.cpp"
  "Entity/Engine/Palette.cpp"
  "Entity/Engine/Surface.cpp"
  "Entity/Engine/Tickable.cpp"
)

set ( entity_game_src
#  "Entity/Game/Base.cpp"
#  "Entity/Game/BasescapeData.cpp"
#  "Entity/Game/BaseFactory.cpp"
#  "Entity/Game/CountryFactory.cpp"
#  "Entity/Game/RegionFactory.cpp"
#  "Entity/Game/UfoFactory.cpp"
)

set ( entity_interface_src
#  "Entity/Interface/ArrowButton.cpp"
  "Entity/Interface/Button.cpp"
  "Entity/Interface/Interface.cpp"
  "Entity/Interface/Text.cpp"
  "Entity/Interface/Window.cpp"
)

set (inspector_src
  "Inspector/CustomPropertyGrid.cpp"
  "Inspector/EntityPanel.cpp"
  "Inspector/Inspector.cpp"
  "Inspector/InspectorFrame.cpp"
  "Inspector/InspectorApp.cpp"
  "Inspector/TypePropertyMapping.cpp"
)

set ( cxx_src
    ${root_src}
    ${basescape_src}
    ${battlescape_src}
    ${engine_src}
    ${geoscape_src}
    ${interface_src}
    ${menu_src}
    ${mod_src}
    ${savegame_src}
    ${ufopedia_src}
    ${lua_src}
    ${entity_common_src}
    ${entity_engine_src}
    ${entity_game_src}
    ${entity_interface_src}
)

set ( c_src
  ../libs/miniz/miniz.c
)
set_property ( SOURCE ${c_src} APPEND PROPERTY COMPILE_DEFINITIONS MINIZ_NO_STDIO )

if(ENABLE_ENTITY_INSPECTOR)
  list(APPEND cxx_src ${inspector_src})
endif()

# All the source files that are necessary to build the project, excluding the main.cpp file
set ( openxcom_src ${c_src} ${cxx_src} ${embed_src} )

set ( c_src
  ../libs/miniz/miniz.c
)
set_property ( SOURCE ${c_src} APPEND PROPERTY COMPILE_DEFINITIONS MINIZ_NO_STDIO )

set(CMAKE_POSITION_INDEPENDENT_CODE ON)


# External libraries
include(FetchContent)

# Fetch YAML
FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG "master" # Can be a tag (yaml-cpp-x.x.x), a commit hash, or a branch name (master)
)
FetchContent_MakeAvailable(yaml-cpp)

# Define custom target for Yaml
add_custom_target(build_yaml
    DEPENDS yaml-cpp::yaml-cpp
    COMMAND ${CMAKE_COMMAND} --build . --target yaml-cpp
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)


# Fetch Lua
#  Lua is used as a scripting language to showcase the script binding capabilities of SimpleRTTR
FetchContent_Declare(
  lua
  GIT_REPOSITORY https://github.com/lua/lua.git
  GIT_TAG "origin/v5.4"
)
FetchContent_MakeAvailable(lua)

# Lua does not provide a CMake build system, so we need to manually set up the target
add_library(lua STATIC  "${lua_SOURCE_DIR}/lapi.c"
                        "${lua_SOURCE_DIR}/lauxlib.c"
                        "${lua_SOURCE_DIR}/lbaselib.c"
                        "${lua_SOURCE_DIR}/lcode.c"
                        "${lua_SOURCE_DIR}/lcorolib.c"
                        "${lua_SOURCE_DIR}/lctype.c"
                        "${lua_SOURCE_DIR}/ldblib.c"
                        "${lua_SOURCE_DIR}/ldebug.c"
                        "${lua_SOURCE_DIR}/ldo.c"
                        "${lua_SOURCE_DIR}/ldump.c"
                        "${lua_SOURCE_DIR}/lfunc.c"
                        "${lua_SOURCE_DIR}/lgc.c"
                        "${lua_SOURCE_DIR}/linit.c"
                        "${lua_SOURCE_DIR}/liolib.c"
                        "${lua_SOURCE_DIR}/llex.c"
                        "${lua_SOURCE_DIR}/lmathlib.c"
                        "${lua_SOURCE_DIR}/lmem.c"
                        "${lua_SOURCE_DIR}/loadlib.c"
                        "${lua_SOURCE_DIR}/lobject.c"
                        "${lua_SOURCE_DIR}/lopcodes.c"
                        "${lua_SOURCE_DIR}/loslib.c"
                        "${lua_SOURCE_DIR}/lparser.c"
                        "${lua_SOURCE_DIR}/lstate.c"
                        "${lua_SOURCE_DIR}/lstring.c"
                        "${lua_SOURCE_DIR}/lstrlib.c"
                        "${lua_SOURCE_DIR}/ltable.c"
                        "${lua_SOURCE_DIR}/ltablib.c"
                        "${lua_SOURCE_DIR}/ltests.c"
                        "${lua_SOURCE_DIR}/ltm.c"
                        "${lua_SOURCE_DIR}/lundump.c"
                        "${lua_SOURCE_DIR}/lutf8lib.c"
                        "${lua_SOURCE_DIR}/lvm.c"
                        "${lua_SOURCE_DIR}/lzio.c")

target_include_directories(lua PUBLIC ${lua_SOURCE_DIR}/src)

# Define custom target for Lua
add_custom_target(build_lua
    DEPENDS lua
    COMMAND ${CMAKE_COMMAND} --build . --target lua
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Fetch EnTT
#  EnTT is used as the Entity Component System library for the game
FetchContent_Declare(
  EnTT
  GIT_REPOSITORY https://github.com/skypjack/entt.git
  GIT_TAG "origin/master"
)
FetchContent_MakeAvailable(EnTT)

# Fetch SimpleRTTR
#  SimpleRTTR is used for reflection in the Entity Component System
set(SIMPLERTTR_BUILD_TESTS OFF CACHE BOOL "Build SimpleRTTR Tests and Examples")
set(SIMPLERTTR_BUILD_EXAMPLES OFF CACHE BOOL "Build SimpleRTTR Tests and Examples")
FetchContent_Declare(
  SimpleRTTR
  GIT_REPOSITORY https://github.com/ken-noland/SimpleRTTR
  GIT_TAG "origin/main"
)
FetchContent_MakeAvailable(SimpleRTTR)

# Fetch nlohmann/json
#  nlohmann/json is used for JSON serialization which is used all over the place
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json
  GIT_TAG "v3.11.3"
)
FetchContent_MakeAvailable(nlohmann_json)

# Fetch inja
#  inja is used for templating for the code generation
FetchContent_Declare(
  inja
  URL https://github.com/pantor/inja/releases/download/v3.4.0/inja.hpp
  DOWNLOAD_NO_EXTRACT TRUE
)
FetchContent_MakeAvailable(inja)

# Fetch Dear ImGui
#  Dear ImGui is used for the entity inspector
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.91.4
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(imgui)

# Define ImGui sources
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
)

# Add ImGui library
add_library(ImGui STATIC ${IMGUI_SOURCES})
target_include_directories(ImGui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)

# Fetch Vulkan-Headers
#  Vulkan-Headers is used for Vulkan API headers
FetchContent_Declare(
    vulkan_headers
    GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
    GIT_TAG        v1.3.296  # Update to the desired version(which is dependent on SPIRV-headers and SPIRV-tools below)
)
FetchContent_MakeAvailable(vulkan_headers)

# Fetch SPIRV-Headers first
#  SPIRV-Headers is a dependency of SPIRV-Tools
FetchContent_Declare(
    spirv-headers
    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Headers.git
    GIT_TAG vulkan-sdk-1.3.296.0
)

FetchContent_MakeAvailable(spirv-headers)

# Fetch SPIRV-Tools
#  SPIRV-Tools is used for compiling SPIR-V shaders
set(SPIRV_SKIP_TESTS ON CACHE BOOL "" FORCE)
set(SPIRV_HEADERS_SKIP_INSTALL ON CACHE BOOL "" FORCE)
set(SPIRV_WERROR OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    spirv-tools
    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Tools.git
    GIT_TAG vulkan-sdk-1.3.296.0
)

# Enable building of SPIRV-Tools as Shaderc requires it
FetchContent_MakeAvailable(spirv-tools)

# Fetch glslang
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE) # Skip building glslangValidator, etc.
set(ENABLE_SPVREMAPPER OFF CACHE BOOL "" FORCE)
set(ENABLE_HLSL ON CACHE BOOL "" FORCE) # Optional, depending on if you need HLSL support

FetchContent_Declare(
    glslang
    GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
    GIT_TAG main
)

FetchContent_MakeAvailable(glslang)


# Fetch Shaderc
#  Shaderc is used for compiling GLSL shaders
set(SHADERC_SKIP_INSTALL ON CACHE BOOL "Skip building ShaderC install")
set(SHADERC_SKIP_TESTS ON CACHE BOOL "Skip building ShaderC tests")
set(SHADERC_SKIP_EXAMPLES ON CACHE BOOL "Skip building ShaderC examples")
set(SHADERC_SKIP_COPYRIGHT_CHECK ON CACHE BOOL "Skip building ShaderC copyright check")

FetchContent_Declare(
    shaderc
    GIT_REPOSITORY https://github.com/google/shaderc.git
    GIT_TAG v2024.3
)

FetchContent_MakeAvailable(shaderc)

# Fetch GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)

FetchContent_MakeAvailable(glm)

# Fetch VulkanMemoryAllocator
#  VulkanMemoryAllocator is used for managing Vulkan memory
FetchContent_Declare(
    VulkanMemoryAllocator
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
    GIT_TAG v3.1.0
)

FetchContent_MakeAvailable(VulkanMemoryAllocator)

# set up the defines and includes
set(openxcom_defines
  YAML_CPP_STATIC_DEFINE
  NOMINMAX
  VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
  VMA_STATIC_VULKAN_FUNCTIONS=0
)

if(MSVC)
  # Add a define which instructs Vulkan to use the Win32 KHR
  list(APPEND openxcom_defines VK_USE_PLATFORM_WIN32_KHR)
elseif(APPLE)
  # Add a define which instructs Vulkan to use the Metal Extensions
  list(APPEND openxcom_defines VK_USE_PLATFORM_METAL_EXT)
elseif(UNIX)
  # Add a define which instructs Vulkan to use the XLIB KHR
  list(APPEND openxcom_defines VK_USE_PLATFORM_XLIB_KHR)
endif()

message(STATUS "VulkanMemoryAllocator_SOURCE_DIR: ${VulkanMemoryAllocator_SOURCE_DIR}")

set(openxcom_includes
  ${yaml-cpp_SOURCE_DIR}/include
  ${lua_SOURCE_DIR}
  ${EnTT_SOURCE_DIR}/single_include
  ${SimpleRTTR_SOURCE_DIR}/include
  ${nlohmann_json_SOURCE_DIR}/include
  ${inja_SOURCE_DIR}
  ${vulkan_headers_SOURCE_DIR}/include
  ${glslang_SOURCE_DIR}
)

set(openxcom_libs
  yaml-cpp::yaml-cpp
  lua
  ImGui
  shaderc
  glm
  GPUOpen::VulkanMemoryAllocator
)

if(UNIX)
  # Find the X11 package
  find_package(X11 REQUIRED)

  # Link the X11 libraries to your target
  if(X11_FOUND)
    list(APPEND openxcom_includes ${X11_INCLUDE_DIR})
    list(APPEND openxcom_libs ${X11_LIBRARIES})
  endif() #X11_FOUND
endif() #UNIX

# openxcom base library targets
add_library(openxcom_base_lib OBJECT ${openxcom_src})
target_include_directories(openxcom_base_lib PUBLIC ${openxcom_includes})
target_compile_definitions(openxcom_base_lib PUBLIC ${openxcom_defines})
target_link_libraries(openxcom_base_lib PUBLIC ${openxcom_libs})

if (MSVC)
    #mod.cpp is so massive that MSVC has trouble keeping it all in a single object, so we need to split it up with this command
    target_compile_options(openxcom_base_lib PRIVATE /bigobj)
endif()

# Codegen: Executable that generates additional source files, depends on openxcom_base_lib
add_executable(codegen "Codegen/Codegen.cpp" $<TARGET_OBJECTS:openxcom_base_lib>)
target_include_directories(codegen PUBLIC ${openxcom_includes})
target_compile_definitions(codegen PUBLIC ${openxcom_defines})
target_link_libraries(codegen PUBLIC ${openxcom_libs})

# Allow undefined symbols for codegen
if(MSVC)
  target_link_options(codegen PRIVATE "/FORCE")
elseif(APPLE)
  target_link_options(codegen PRIVATE "-undefined dynamic_lookup")
elseif(UNIX)
  target_link_options(codegen PRIVATE "-Wl,--unresolved-symbols=ignore-in-object-files")
  target_link_options(codegen PRIVATE "-no-pie")
  target_compile_options(codegen PRIVATE -fPIC)
endif()

# Define the generated source files
set(generated_sources
    "${CMAKE_BINARY_DIR}/generated/json.cpp"
    # Add more generated files as needed
)

set(codegen_templates
    "Codegen/Templates/Json/Json.cpp.inja"
    "Codegen/Templates/Json/JsonEnum.cpp.inja"
    "Codegen/Templates/Json/JsonHeader.cpp.inja"
    "Codegen/Templates/Json/JsonMap.cpp.inja"
    "Codegen/Templates/Json/JsonObject.cpp.inja"
    "Codegen/Templates/Json/JsonOption.cpp.inja"
    "Codegen/Templates/Json/JsonVector.cpp.inja"
    "Codegen/Templates/Json/JsonTemplate.json"
)

add_custom_target(create_generated_dir ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/generated"
    COMMENT "Creating generated directory"
)

# Custom command to generate the source files using codegen
add_custom_command(
    OUTPUT "${CMAKE_BINARY_DIR}/generated/json.cpp"
    COMMAND $<TARGET_FILE:codegen> -template "${CMAKE_CURRENT_SOURCE_DIR}/Codegen/Templates/Json/JsonTemplate.json" -output "${CMAKE_BINARY_DIR}/generated/json.cpp"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/generated
    DEPENDS create_generated_dir codegen ${codegen_templates}
    COMMENT "Generating source files with codegen"
)

# openxcom_lib: Library compiled from generated source files, depends on openxcom_base_lib
add_library(openxcom_lib STATIC ${generated_sources} $<TARGET_OBJECTS:openxcom_base_lib>)
add_dependencies(openxcom_lib codegen) # Ensure codegen runs before compiling openxcom_lib
target_include_directories(openxcom_lib PUBLIC ${openxcom_includes})
target_compile_definitions(openxcom_lib PUBLIC ${openxcom_defines})
target_link_libraries(openxcom_lib PUBLIC ${openxcom_libs})

add_executable(openxcom WIN32 main.cpp ${openxcom_icon})
target_link_libraries(openxcom PUBLIC openxcom_lib)

# Add the tests subdirectory
add_subdirectory(Tests)
