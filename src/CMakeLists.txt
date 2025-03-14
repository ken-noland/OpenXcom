# CMakeLists.txt

# CMake build type
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type specified. Defaulting to Debug build.")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type (Debug, Release, etc.)" FORCE)
endif()

# Define option to enable or disable the EntityInspector
option(ENABLE_ENTITY_INSPECTOR "Enable the EntityInspector for debugging and editing entities." OFF)

# Force fetch of github dependencies
option(FORCE_FETCH "Force re-download of FetchContent dependencies" OFF)

if(FORCE_FETCH)
  # When forcing a re-download, let FetchContent check for updates
  set(FETCHCONTENT_UPDATES_DISCONNECTED OFF CACHE BOOL "Force updating of fetched content" FORCE)
else()
  # Otherwise, assume you are offline or want to use the local copy
  set(FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE BOOL "Disable update check for fetched content" FORCE)
endif()

set ( root_src
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
  # need to include this first so that the definitions for glm are available
  "Engine/Utility/RTTRGlm.cpp"

#  "Engine/Adlib/adlplayer.cpp"
#  "Engine/Adlib/fmopl.cpp"
  "Engine/Filesystem/CompositeFileSystem.cpp"
  "Engine/Filesystem/EmbeddedFileSystem.cpp"
  "Engine/Filesystem/PhysicalFileSystem.cpp"
  "Engine/Filesystem/VirtualFileSystem.cpp"
  "Engine/Graphics/Common/GameSurface.cpp"
  "Engine/Graphics/Common/WindowSurface.cpp"
  "Engine/Graphics/Font/Font.cpp"
  "Engine/Graphics/Font/FontManager.cpp"
  "Engine/Graphics/Font/FontPack.cpp"
  "Engine/Graphics/Image/Image.cpp"
  "Engine/Graphics/Image/ImageManager.cpp"
  "Engine/Graphics/Palette/Palette.cpp"
  "Engine/Graphics/Palette/PaletteManager.cpp"
  "Engine/Graphics/Primitive/BoxPrimitive.cpp"
  "Engine/Graphics/Primitive/BoxPrimitiveFactory.cpp"
  "Engine/Graphics/Primitive/ImagePrimitive.cpp"
  "Engine/Graphics/Primitive/ImagePrimitiveFactory.cpp"
  "Engine/Graphics/Primitive/LinePrimitive.cpp"
  "Engine/Graphics/Primitive/LinePrimitiveFactory.cpp"
  "Engine/Graphics/Primitive/PointPrimitive.cpp"
  "Engine/Graphics/Primitive/PointPrimitiveFactory.cpp"
  "Engine/Graphics/Primitive/PrimitiveFactory.cpp"
  "Engine/Graphics/Primitive/ShaderCollection.cpp"
  "Engine/Graphics/Primitive/TextPrimitive.cpp"
  "Engine/Graphics/Primitive/TextPrimitiveFactory.cpp"
  "Engine/Graphics/Types/PackedColor.cpp"
  "Engine/Graphics/Vulkan/Buffer/VulkanBuffer.cpp"
  "Engine/Graphics/Vulkan/Buffer/VulkanBufferManager.cpp"
  "Engine/Graphics/Vulkan/Image/VulkanImage.cpp"
  "Engine/Graphics/Vulkan/Image/VulkanImageManager.cpp"
  "Engine/Graphics/Vulkan/Image/VulkanRenderTarget.cpp"
  "Engine/Graphics/Vulkan/Pipeline/VulkanPipeline.cpp"
  "Engine/Graphics/Vulkan/Pipeline/VulkanPipelineBinding.cpp"
  "Engine/Graphics/Vulkan/Pipeline/VulkanPipelineFactory.cpp"
  "Engine/Graphics/Vulkan/Pipeline/VulkanPipelineManager.cpp"
  "Engine/Graphics/Vulkan/Shader/VulkanShader.cpp"
  "Engine/Graphics/Vulkan/Shader/VulkanShaderManager.cpp"
  "Engine/Graphics/Vulkan/Surface/VulkanWindowedSurface.cpp"
  "Engine/Graphics/Vulkan/Surface/VulkanHeadlessSurface.cpp"
  "Engine/Graphics/Vulkan/VMAImplementation.cpp"
  "Engine/Graphics/Vulkan/VulkanCommand.cpp"
  "Engine/Graphics/Vulkan/VulkanContext.cpp"
  "Engine/Graphics/Vulkan/VulkanDescriptorSet.cpp"
  "Engine/Graphics/Vulkan/VulkanQueue.cpp"
  "Engine/Graphics/Vulkan/VulkanSampler.cpp"
  "Engine/Graphics/Vulkan/VulkanSystem.cpp"
  "Engine/Graphics/Shader.cpp"
  "Engine/Graphics/ShaderManager.cpp"
  "Engine/Graphics/GraphicsSystem.cpp"
  "Engine/Mod/Loader/Current/ModLoader.cpp"
  "Engine/Mod/Loader/8.1.2/ModLoader.cpp"
  "Engine/Mod/Mod.cpp"
  "Engine/Mod/ModInfo.cpp"
  "Engine/Mod/ModScanner.cpp"
  "Engine/Platform/Linux/LinuxProcess.cpp"
  "Engine/Platform/Linux/LinuxWindow.cpp"
  "Engine/Platform/Win32/Win32Process.cpp"
  "Engine/Platform/Win32/Win32Window.cpp"
  "Engine/Platform/PlatformProcessSystem.cpp"
  "Engine/Platform/PlatformWindow.cpp"
  "Engine/Platform/PlatformWindowSystem.cpp"
  "Engine/Resource/FileProcessor/ImageBDYFileProcessor.cpp"
  "Engine/Resource/FileProcessor/ImageBMPFileProcessor.cpp"
  "Engine/Resource/FileProcessor/ImageFileProcessor.cpp"
  "Engine/Resource/FileProcessor/ImagePCKFileProcessor.cpp"
  "Engine/Resource/FileProcessor/ImagePNGFileProcessor.cpp"
  "Engine/Resource/FileProcessor/ImageSCRFileProcessor.cpp"
  "Engine/Resource/FileProcessor/ImageSPKFileProcessor.cpp"
  "Engine/Resource/FileProcessor/PaletteDATFileProcessor.cpp"
  "Engine/Resource/ResourceSystem.cpp"
  "Engine/Utility/Time/TimeSystem.cpp"
#  "Engine/AdlibMusic.cpp"
#  "Engine/CatFile.cpp"
  "Engine/CrossPlatform.cpp"
  "Engine/Engine.cpp"
  "Engine/Exception.cpp"
#  "Engine/FastLineClip.cpp"
#  "Engine/FileMap.cpp"
#  "Engine/FlcPlayer.cpp"
#  "Engine/Font.cpp"
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
#  "Engine/Surface.cpp"
#  "Engine/SurfaceSet.cpp"
  "Engine/Timer.cpp"
  "Engine/Unicode.cpp"
  "Engine/Yaml.cpp"
  "Engine/YamlAssert.cpp"
#  "Engine/Zoom.cpp
)

set( game_src
  "Game/Game.cpp"
  "Game/GameMods.cpp"
  "Game/GameStates.cpp"
  "Game/GameWindow.cpp"
  "Game/Resource/MasterFileProcessor.cpp"
  "Game/Resource/Xcom1FileProcessor.cpp"
  "Game/Resource/Xcom2FileProcessor.cpp"
  "Game/States/StartState.cpp"
  "Game/States/State.cpp"
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
    ${game_src}
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

if(ENABLE_ENTITY_INSPECTOR)
  list(APPEND cxx_src ${inspector_src})
endif()

# All the source files that are necessary to build the project, excluding the main.cpp file
set ( openxcom_src ${c_src} ${cxx_src} ${embed_src} )

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# GCC/Clang specific compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    message(STATUS "Configuring for GCC/Clang in Debug mode")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
endif()

#--------------------------------
# External libraries
include(FetchContent)

include(cmake/yaml.cmake)
include(cmake/lua.cmake)
include(cmake/entt.cmake)
include(cmake/nlohmann_json.cmake)
include(cmake/simplerttr.cmake)
include(cmake/inja.cmake)
include(cmake/imgui.cmake)
include(cmake/vulkan-headers.cmake)
include(cmake/spirv-headers.cmake)
include(cmake/spirv-tools.cmake)
include(cmake/glslang.cmake)
include(cmake/shaderc.cmake)
include(cmake/glm.cmake)
include(cmake/vulkanmemoryallocator.cmake)
include(cmake/lodepng.cmake)
include(cmake/libbmp.cmake)
include(cmake/harfbuzz.cmake)
include(cmake/libunibreak.cmake)
include(cmake/utfcpp.cmake)
include(cmake/semver.cmake)

#--------------------------------
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

set(openxcom_includes
  ${yaml-cpp_SOURCE_DIR}/include
  ${rapidyaml_SOURCE_DIR}/src
  ${lua_SOURCE_DIR}
  ${EnTT_SOURCE_DIR}/single_include
  ${SimpleRTTR_SOURCE_DIR}/include
  ${nlohmann_json_SOURCE_DIR}/include
  ${inja_SOURCE_DIR}
  ${vulkan_headers_SOURCE_DIR}/include
  ${glslang_SOURCE_DIR}
  ${lodepng_SOURCE_DIR}
  ${libbmp_SOURCE_DIR}
  ${libunibreak_SOURCE_DIR}/src
  ${utfcpp_SOURCE_DIR}/source
)

set(openxcom_libs
  yaml-cpp::yaml-cpp
  ryml::ryml
  lua
  ImGui
  shaderc
  glm
  GPUOpen::VulkanMemoryAllocator
  lodepng
  libbmp
  harfbuzz
  libunibreak
  semver
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
add_library(openxcom_base_lib STATIC ${openxcom_src})
target_include_directories(openxcom_base_lib PUBLIC ${openxcom_includes})
target_compile_definitions(openxcom_base_lib PUBLIC ${openxcom_defines})
target_link_libraries(openxcom_base_lib PUBLIC ${openxcom_libs})

if (MSVC)
    #mod.cpp is so massive that MSVC has trouble keeping it all in a single object, so we need to split it up with this command
    target_compile_options(openxcom_base_lib PRIVATE /bigobj /utf-8 /wd4828)
elseif (UNIX)
    # Enable all warnings for the base library
    # -Wno-nullability-completeness add a target compiler option to disable warnings for VMA
    target_compile_options(openxcom_base_lib PRIVATE -Wno-nullability-completeness)
endif()

# Codegen: Executable that generates additional source files, depends on openxcom_base_lib
add_executable(codegen "Codegen/Codegen.cpp" $<TARGET_OBJECTS:openxcom_base_lib>)
target_include_directories(codegen PUBLIC ${openxcom_includes})
target_compile_definitions(codegen PUBLIC ${openxcom_defines} SOURCE_DIR="${CMAKE_SOURCE_DIR}")
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
    "${CMAKE_BINARY_DIR}/generated/yaml.cpp"
    # Add more generated files as needed
)

set(json_codegen_templates
    "Codegen/Templates/Json/Json.cpp.inja"
    "Codegen/Templates/Json/JsonEnum.cpp.inja"
    "Codegen/Templates/Json/JsonHeader.cpp.inja"
    "Codegen/Templates/Json/JsonMap.cpp.inja"
    "Codegen/Templates/Json/JsonObject.cpp.inja"
    "Codegen/Templates/Json/JsonOption.cpp.inja"
    "Codegen/Templates/Json/JsonPair.cpp.inja"
    "Codegen/Templates/Json/JsonVector.cpp.inja"
    "Codegen/Templates/Json/JsonTemplate.json"
)

set(yaml_codegen_templates
    "Codegen/Templates/Yaml/Yaml.cpp.inja"
    "Codegen/Templates/Yaml/YamlEnum.cpp.inja"
    "Codegen/Templates/Yaml/YamlHeader.cpp.inja"
    "Codegen/Templates/Yaml/YamlMap.cpp.inja"
    "Codegen/Templates/Yaml/YamlObject.cpp.inja"
    "Codegen/Templates/Yaml/YamlOption.cpp.inja"
    "Codegen/Templates/Yaml/YamlPair.cpp.inja"
    "Codegen/Templates/Yaml/YamlVector.cpp.inja"
    "Codegen/Templates/Yaml/YamlTemplate.json"
)

add_custom_target(create_generated_dir ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/generated"
    COMMENT "Creating generated directory"
)


# Custom command to generate the source files using codegen

# Json
add_custom_command(
    OUTPUT "${CMAKE_BINARY_DIR}/generated/json.cpp"
    COMMAND $<TARGET_FILE:codegen> -template "${CMAKE_CURRENT_SOURCE_DIR}/Codegen/Templates/Json/JsonTemplate.json" -output "${CMAKE_BINARY_DIR}/generated/json.cpp"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/generated
    DEPENDS create_generated_dir codegen ${json_codegen_templates}
    COMMENT "Generating Json source files with codegen"
)

# Yaml
add_custom_command(
    OUTPUT "${CMAKE_BINARY_DIR}/generated/yaml.cpp"
    COMMAND $<TARGET_FILE:codegen> -template "${CMAKE_CURRENT_SOURCE_DIR}/Codegen/Templates/Yaml/YamlTemplate.json" -output "${CMAKE_BINARY_DIR}/generated/yaml.cpp"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/generated
    DEPENDS create_generated_dir codegen ${yaml_codegen_templates}
    COMMENT "Generating Yaml source files with codegen"
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(
        ${CMAKE_BINARY_DIR}/generated/json.cpp
        ${CMAKE_BINARY_DIR}/generated/yaml.cpp
        PROPERTIES COMPILE_FLAGS "-Wno-extra-qualification"
    )
endif()

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
