// Don't worry about Intellisense errors here, as this file is only used in conjunction with Options.h/Options.cpp
// To add a new option, add a new variable entry and a corresponding OptionInfo in Options.cpp

// General options
OPT int displayWidth, displayHeight, maxFrameSkip, baseXResolution, baseYResolution, baseXGeoscape, baseYGeoscape, baseXBattlescape, baseYBattlescape,
	soundVolume, musicVolume, uiVolume, audioSampleRate, audioBitDepth, audioChunkSize, pauseMode, windowedModePositionX, windowedModePositionY, FPS, FPSInactive,
	changeValueByMouseWheel, dragScrollTimeTolerance, dragScrollPixelTolerance, mousewheelSpeed, autosaveFrequency;
OPT bool fullscreen, asyncBlit, playIntro, useScaleFilter, useHQXFilter, useXBRZFilter, useOpenGL, checkOpenGLErrors, vSyncForOpenGL, useOpenGLSmoothing,
	autosave, allowResize, borderless, debug, debugUi, fpsCounter, newSeedOnLoad, keepAspectRatio, nonSquarePixelRatio,
	cursorInBlackBandsInFullscreen, cursorInBlackBandsInWindow, cursorInBlackBandsInBorderlessWindow, maximizeInfoScreens, musicAlwaysLoop, StereoSound, verboseLogging, soldierDiaries, touchEnabled,
	rootWindowedMode, lazyLoadResources, backgroundMute, showCraftHangar;
OPT std::string language, useOpenGLShader;
OPT KeyboardType keyboardMode;
OPT SaveSort saveOrder;
OPT MusicFormat preferredMusic;
OPT SoundFormat preferredSound;
OPT VideoFormat preferredVideo;
OPT SDL_GrabMode captureMouse;
OPT TextWrapping wordwrap;
OPT SDLKey keyOk, keyCancel, keyScreenshot, keyFps, keyQuickLoad, keyQuickSave;

// Geoscape options
OPT int geoClockSpeed, dogfightSpeed, geoScrollSpeed, geoDragScrollButton, geoscapeScale;
OPT bool includePrimeStateInSavedLayout, anytimePsiTraining, weaponSelfDestruction, retainCorpses, craftLaunchAlways, dogfightAI,
	globeSurfaceCache, globeSeasons, globeDetail, globeRadarLines, globeFlightPaths, globeAllRadarsOnBaseBuild,
	storageLimitsEnforced, canSellLiveAliens, canTransferCraftsWhileAirborne, customInitialBase, aggressiveRetaliation, geoDragScrollInvert,
	allowBuildingQueue, showFundsOnGeoscape, psiStrengthEval, allowPsiStrengthImprovement, fieldPromotions, meetingPoint;
OPT SDLKey keyGeoLeft, keyGeoRight, keyGeoUp, keyGeoDown, keyGeoZoomIn, keyGeoZoomOut, keyGeoSpeed1, keyGeoSpeed2, keyGeoSpeed3, keyGeoSpeed4, keyGeoSpeed5, keyGeoSpeed6,
	keyGeoIntercept, keyGeoBases, keyGeoGraphs, keyGeoUfopedia, keyGeoOptions, keyGeoFunding, keyGeoToggleDetail, keyGeoToggleRadar,
	keyBaseSelect1, keyBaseSelect2, keyBaseSelect3, keyBaseSelect4, keyBaseSelect5, keyBaseSelect6, keyBaseSelect7, keyBaseSelect8;
OPT SDLKey keyAIList;	

// Battlescape options
OPT ScrollType battleEdgeScroll;
OPT PathPreview battleNewPreviewPath;
OPT int battleRealisticShotDispersion, battleScrollSpeed, battleDragScrollButton, battleFireSpeed, battleXcomSpeed, battleAlienSpeed, battleExplosionHeight, battlescapeScale, battleTerrainSquishyness;
OPT bool traceAI, battleInstantGrenade, battleNotifyDeath, battleTooltips, battleHairBleach, battleAutoEnd,
	strafe, forceFire, showMoreStatsInInventoryView, allowPsionicCapture, skipNextTurnScreen, disableAutoEquip, battleDragScrollInvert,
	battleUFOExtenderAccuracy, battleRealisticAccuracy, battleRealisticImprovedSnap, battleRealisticDisplayRolls, battleConfirmFireMode, battleSmoothCamera, noAlienPanicMessages, alienBleeding, instantPrime, strictBlockedChecking;
OPT SDLKey keyBattleLeft, keyBattleRight, keyBattleUp, keyBattleDown, keyBattleLevelUp, keyBattleLevelDown, keyBattleCenterUnit, keyBattlePrevUnit, keyBattleNextUnit, keyBattleDeselectUnit,
keyBattleUseLeftHand, keyBattleUseRightHand, keyBattleInventory, keyBattleMap, keyBattleOptions, keyBattleEndTurn, keyBattleAbort, keyBattleStats, keyBattleKneel,
keyBattleReserveKneel, keyBattleReload, keyBattlePersonalLighting, keyBattleReserveNone, keyBattleReserveSnap, keyBattleReserveAimed, keyBattleReserveAuto,
keyBattleCenterEnemy1, keyBattleCenterEnemy2, keyBattleCenterEnemy3, keyBattleCenterEnemy4, keyBattleCenterEnemy5, keyBattleCenterEnemy6, keyBattleCenterEnemy7, keyBattleCenterEnemy8,
keyBattleCenterEnemy9, keyBattleCenterEnemy10, keyBattleVoxelView, keyBattleZeroTUs, keyInvCreateTemplate, keyInvApplyTemplate, keyInvClear, keyInvAutoEquip;

// AI options
OPT bool sneakyAI, brutalAI, brutalBrutes, cheatOnMovement, brutalCivilians, ignoreDelay, allowPreprime, autoCombat, aiPerformanceOptimization, avoidMines, avoidCuddle;
OPT int aiTargetMode, aiAggression, aiCarefulness, autoAggression, autoCarefulness, aggressionMode, intelligenceMode, intelligence, aiPreset;
OPT bool autoCombatEachCombat, autoCombatEachTurn, autoCombatControlPerUnit;

// Extra hotkeys (OXCE)
OPT SDLKey keyGeoDailyPilotExperience, keyGeoUfoTracker, keyGeoTechTreeViewer, keyGeoGlobalResearch, keyGeoGlobalProduction, keyGeoGlobalAlienContainment,
	keyGraphsZoomIn, keyGraphsZoomOut,
	keyToggleQuickSearch,
	keyCraftLoadoutSave, keyCraftLoadoutLoad,
	keyMarkAllAsSeen,
	keySellAll, keySellAllButOne,
	keyTransferAll,
	keyRemoveSoldiersFromCraft, keyRemoveSoldiersFromAllCrafts,
	keyRemoveEquipmentFromCraft,
	keyRemoveArmorFromCraft, keyRemoveArmorFromAllCrafts,
	keyRemoveSoldiersFromTraining, keyAddSoldiersToTraining,
	keyInventoryArmor, keyInventoryAvatar, keyInventorySave, keyInventoryLoad,
	keyInvSavePersonalEquipment, keyInvLoadPersonalEquipment, keyInvShowPersonalEquipment,
	keyBattleShowLayers,
	keyBattleUseSpecial,
	keyBattleActionItem1, keyBattleActionItem2, keyBattleActionItem3, keyBattleActionItem4, keyBattleActionItem5,
	keyNightVisionToggle, keyNightVisionHold, keySelectMusicTrack, keyToggleAutoPlay;
OPT SDLKey keyBasescapeBuildNewBase, keyBasescapeBaseInfo, keyBasescapeSoldiers, keyBasescapeCrafts,
	keyBasescapeFacilities, keyBasescapeResearch, keyBasescapeManufacture, keyBasescapeTransfer,
	keyBasescapePurchase, keyBasescapeSell;

// OXCE, accessible via GUI
OPT bool oxceUpdateCheck;
OPT int autosaveSlots;
OPT bool oxceManualPromotions;
OPT bool oxceAutomaticPromotions;

OPT bool oxceLinks;
OPT bool oxceAlternateCraftEquipmentManagement;
OPT bool oxceUfoLandingAlert;
OPT int oxceWoundedDefendBaseIf;
OPT bool oxcePlayBriefingMusicDuringEquipment;
OPT int oxceNightVisionColor;
OPT int oxceFOW;
OPT int oxceFOWColor;
OPT bool oxceAutoSell;
OPT int oxceAutoNightVisionThreshold;
OPT bool oxceRememberDisabledCraftWeapons;
OPT bool oxceEnableOffCentreShooting;
OPT bool oxceSmartCtrlEquip;

// OXCE hidden, accessible only via options.cfg
OPT bool oxceFatFingerLinks;
OPT int oxceThrottleMouseMoveEvent;
OPT bool oxceHighlightNewTopicsHidden;
OPT int oxceInterceptGuiMaintenanceTimeHidden;
OPT bool oxceEnableUnitResponseSounds;
OPT bool oxceEnableSlackingIndicator;
OPT bool oxceEnablePaletteFlickerFix;
OPT int oxceMaxEquipmentLayoutTemplates;
OPT bool oxcePersonalLayoutIncludingArmor;
OPT bool oxceManufactureFilterSuppliesOK;
// 0 = not persisted; 1 = persisted per battle; 2 = persisted per campaign
OPT int oxceTogglePersonalLightType;
OPT int oxceToggleNightVisionType;
OPT int oxceToggleBrightnessType;
OPT int maxNumberOfBases;
/**
 * Verification level of mod data.
 * Same levels supported as `SeverityLevel`.
 */
OPT int oxceModValidationLevel;

OPT bool oxceEmbeddedOnly;
OPT bool oxceListVFSContents;
OPT bool oxceRawScreenShots;
OPT bool oxceFirstPersonViewFisheyeProjection;
OPT bool oxceThumbButtons;

OPT bool oxceRecommendedOptionsWereSet;
OPT std::string password;

// OXCE hidden, but moddable via fixedUserOptions and/or recommendedUserOptions
OPT int oxceStartUpTextMode;
OPT int oxceManufactureScrollSpeed;
OPT int oxceManufactureScrollSpeedWithCtrl;
OPT int oxceResearchScrollSpeed;
OPT int oxceResearchScrollSpeedWithCtrl;
OPT int oxceGeoSlowdownFactor;
OPT bool oxceDisableTechTreeViewer;
OPT bool oxceDisableStatsForNerds;
OPT bool oxceDisableProductionDependencyTree;
OPT bool oxceDisableHitLog;
OPT bool oxceDisableAlienInventory;
OPT bool oxceDisableInventoryTuCost;
OPT bool oxceShowBaseNameInPopups;
OPT int oxceGeoscapeDebugLogMaxEntries;
OPT bool oxceGeoscapeEventsInstantDelivery;

// Flags and other stuff that don't need OptionInfo's.
OPT bool mute, reload, newOpenGL, newScaleFilter, newHQXFilter, newXBRZFilter, newRootWindowedMode, newFullscreen, newAllowResize, newBorderless;
OPT int newDisplayWidth, newDisplayHeight, newBattlescapeScale, newGeoscapeScale, newWindowedModePositionX, newWindowedModePositionY;
OPT std::string newOpenGLShader;
OPT std::vector< std::pair<std::string, bool> > mods; // ordered list of available mods (lowest priority to highest) and whether they are active
OPT SoundFormat currentSound;
