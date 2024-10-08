cmake_minimum_required(VERSION 3.15)

################################################################################

project(CryMP-ATOM-Server LANGUAGES C CXX RC)

################################################################################

if(NOT MSVC)
	message(FATAL_ERROR "MSVC is the only supported compiler!")
endif()

if(NOT CMAKE_MSVC_RUNTIME_LIBRARY)
	# use statically linked MSVC runtime library
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

if(MSVC_IDE)
	# enable Edit and Continue
	add_compile_options(/ZI)
endif()

################################################################################

option(CRYMP_CONSOLE_APP "Enable console window with standard output." OFF)

option(CRYMP_USE_MIMALLOC "Use mimalloc as global allocator." OFF)

option(CRYMP_DEBUG_ALLOCATOR_ENABLED "Enable debug allocator to detect memory errors." OFF)
option(CRYMP_DEBUG_ALLOCATOR_VERBOSE "Log each allocation and deallocation to standard output." OFF)
option(CRYMP_DEBUG_ALLOCATOR_OVERFLOW_INSTEAD_OF_UNDERFLOW "Detect buffer overflows instead of underflows." OFF)
option(CRYMP_DEBUG_ALLOCATOR_CHECK_READS "Check for both invalid reads and writes." OFF)

option(CRYMP_TRACY_ENABLED "Enable Tracy profiler." OFF)

################################################################################

math(EXPR BUILD_BITS "${CMAKE_SIZEOF_VOID_P} * 8")

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Build bits: ${BUILD_BITS}-bit")

################################################################################

find_package(Git REQUIRED)

include(CMake/GenerateVersion.cmake)

add_custom_target(GenerateVersion
	${CMAKE_COMMAND}
		-D GIT_EXECUTABLE=${GIT_EXECUTABLE}
		-D PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}
		-D PROJECT_BINARY_DIR=${PROJECT_BINARY_DIR}
		-P ${PROJECT_SOURCE_DIR}/CMake/GenerateVersion.cmake
	VERBATIM
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	BYPRODUCTS ${PROJECT_BINARY_DIR}/version.h
)

message(STATUS "Version number: ${CRYMP_SERVER_VERSION}")
message(STATUS "Version string: ${CRYMP_SERVER_VERSION_STRING}")

################################################################################

set(CRYMP_SERVER_EXE "CryMP-Server${BUILD_BITS}")

message(STATUS "Target executable: ${CRYMP_SERVER_EXE}.exe")

add_executable(${CRYMP_SERVER_EXE} WIN32
	Code/Cry3DEngine/TimeOfDay.cpp
	Code/Cry3DEngine/TimeOfDay.h
	Code/CryCommon/Cry3DEngine/CGF/CGFContent.h
	Code/CryCommon/Cry3DEngine/CGF/CryCompiledFile.h
	Code/CryCommon/Cry3DEngine/CGF/CryHeaders.h
	Code/CryCommon/Cry3DEngine/CGF/IChunkFile.h
	Code/CryCommon/Cry3DEngine/CryEngineDecalInfo.h
	Code/CryCommon/Cry3DEngine/CryParticleSpawnInfo.h
	Code/CryCommon/Cry3DEngine/CryVertexBinding.h
	Code/CryCommon/Cry3DEngine/I3DEngine.h
	Code/CryCommon/Cry3DEngine/I3DSampler.h
	Code/CryCommon/Cry3DEngine/IFoliage.h
	Code/CryCommon/Cry3DEngine/IIndexedMesh.h
	Code/CryCommon/Cry3DEngine/ILMSerializationManager.h
	Code/CryCommon/Cry3DEngine/IMaterial.h
	Code/CryCommon/Cry3DEngine/ISplines.h
	Code/CryCommon/Cry3DEngine/IStatObj.h
	Code/CryCommon/Cry3DEngine/ISurfaceType.h
	Code/CryCommon/Cry3DEngine/LMCompStructures.h
	Code/CryCommon/Cry3DEngine/ParticleParams.h
	Code/CryCommon/CryAISystem/AIFormationDescriptor.h
	Code/CryCommon/CryAISystem/AITrackPatternDescriptor.h
	Code/CryCommon/CryAISystem/AgentParams.h
	Code/CryCommon/CryAISystem/IAIAction.h
	Code/CryCommon/CryAISystem/IAIGroup.h
	Code/CryCommon/CryAISystem/IAIRecorder.h
	Code/CryCommon/CryAISystem/IAISystem.h
	Code/CryCommon/CryAISystem/IAgent.h
	Code/CryCommon/CryAISystem/IInterestSystem.h
	Code/CryCommon/CryAction/IActionMapManager.h
	Code/CryCommon/CryAction/IActorSystem.h
	Code/CryCommon/CryAction/IAnimatedCharacter.h
	Code/CryCommon/CryAction/IAnimationGraph.h
	Code/CryCommon/CryAction/IAnimationGraphSystem.h
	Code/CryCommon/CryAction/IAnimationStateNode.h
	Code/CryCommon/CryAction/IDebrisMgr.h
	Code/CryCommon/CryAction/IDebugHistory.h
	Code/CryCommon/CryAction/IDialogSystem.h
	Code/CryCommon/CryAction/IEffectSystem.h
	Code/CryCommon/CryAction/IFlowSystem.h
	Code/CryCommon/CryAction/IGameFramework.h
	Code/CryCommon/CryAction/IGameObject.h
	Code/CryCommon/CryAction/IGameObjectSystem.h
	Code/CryCommon/CryAction/IGameRulesSystem.h
	Code/CryCommon/CryAction/IGameplayRecorder.h
	Code/CryCommon/CryAction/IInteractor.h
	Code/CryCommon/CryAction/IItem.h
	Code/CryCommon/CryAction/IItemSystem.h
	Code/CryCommon/CryAction/ILevelSystem.h
	Code/CryCommon/CryAction/ILoadGame.h
	Code/CryCommon/CryAction/IMaterialEffects.h
	Code/CryCommon/CryAction/IMovementController.h
	Code/CryCommon/CryAction/IPlayerProfiles.h
	Code/CryCommon/CryAction/ISaveGame.h
	Code/CryCommon/CryAction/ISubtitleManager.h
	Code/CryCommon/CryAction/IUIDraw.h
	Code/CryCommon/CryAction/IVehicleSystem.h
	Code/CryCommon/CryAction/IViewSystem.h
	Code/CryCommon/CryAction/IWeapon.h
	Code/CryCommon/CryAction/IWorldQuery.h
	Code/CryCommon/CryAction/Network/NetActionSync.h
	Code/CryCommon/CryAnimation/CryAnimationScriptCommands.h
	Code/CryCommon/CryAnimation/CryCharAnimationParams.h
	Code/CryCommon/CryAnimation/CryCharMorphParams.h
	Code/CryCommon/CryAnimation/ICryAnimation.h
	Code/CryCommon/CryAnimation/IFacialAnimation.h
	Code/CryCommon/CryAnimation/IJoystick.h
	Code/CryCommon/CryCore/ConfigurableVariant.h
	Code/CryCommon/CryCore/CryArray.h
	Code/CryCommon/CryCore/CryFixedString.h
	Code/CryCommon/CryCore/CryMalloc.h
	Code/CryCommon/CryCore/CryName.h
	Code/CryCommon/CryCore/CryPodArray.h
	Code/CryCommon/CryCore/CrySizer.h
	Code/CryCommon/CryCore/CryString.h
	Code/CryCommon/CryCore/Endian.h
	Code/CryCommon/CryCore/MSVCspecific.h
	Code/CryCommon/CryCore/MiniQueue.h
	Code/CryCommon/CryCore/MultiThread.h
	Code/CryCommon/CryCore/PoolAllocator.h
	Code/CryCommon/CryCore/PoolAllocatorSynchronization.h
	Code/CryCommon/CryCore/StlUtils.h
	Code/CryCommon/CryCore/StringUtils.h
	Code/CryCommon/CryCore/TArray.h
	Code/CryCommon/CryCore/Typelist.h
	Code/CryCommon/CryCore/TypelistUtils.h
	Code/CryCommon/CryCore/VectorMap.h
	Code/CryCommon/CryCore/VectorSet.h
	Code/CryCommon/CryCore/crc32.h
	Code/CryCommon/CryCore/functor.h
	Code/CryCommon/CryCore/platform.h
	Code/CryCommon/CryCore/smartptr.h
	Code/CryCommon/CryEntitySystem/EntityId.h
	Code/CryCommon/CryEntitySystem/IEntity.h
	Code/CryCommon/CryEntitySystem/IEntityClass.h
	Code/CryCommon/CryEntitySystem/IEntityProxy.h
	Code/CryCommon/CryEntitySystem/IEntityRenderState.h
	Code/CryCommon/CryEntitySystem/IEntitySerialize.h
	Code/CryCommon/CryEntitySystem/IEntitySystem.h
	Code/CryCommon/CryFont/IFont.h
	Code/CryCommon/CryGame/CoherentValue.h
	Code/CryCommon/CryGame/GameUtils.h
	Code/CryCommon/CryGame/IGame.h
	Code/CryCommon/CryGame/IGameRef.h
	Code/CryCommon/CryGame/IGameStartup.h
	Code/CryCommon/CryGame/IGameTokens.h
	Code/CryCommon/CryInput/IInput.h
	Code/CryCommon/CryMath/AABBSV.h
	Code/CryCommon/CryMath/Cry_Camera.h
	Code/CryCommon/CryMath/Cry_Color.h
	Code/CryCommon/CryMath/Cry_Geo.h
	Code/CryCommon/CryMath/Cry_GeoDistance.h
	Code/CryCommon/CryMath/Cry_GeoIntersect.h
	Code/CryCommon/CryMath/Cry_GeoOverlap.h
	Code/CryCommon/CryMath/Cry_Math.h
	Code/CryCommon/CryMath/Cry_Matrix.h
	Code/CryCommon/CryMath/Cry_Quat.h
	Code/CryCommon/CryMath/Cry_ValidNumber.h
	Code/CryCommon/CryMath/Cry_Vector2.h
	Code/CryCommon/CryMath/Cry_Vector3.h
	Code/CryCommon/CryMath/GeomQuery.h
	Code/CryCommon/CryMath/Range.h
	Code/CryCommon/CryMovie/AnimKey.h
	Code/CryCommon/CryMovie/IMovieSystem.h
	Code/CryCommon/CryNetwork/INetwork.h
	Code/CryCommon/CryNetwork/INetworkService.h
	Code/CryCommon/CryNetwork/IRemoteControl.h
	Code/CryCommon/CryNetwork/ISerialize.h
	Code/CryCommon/CryNetwork/ISimpleHttpServer.h
	Code/CryCommon/CryNetwork/NetHelpers.h
	Code/CryCommon/CryNetwork/SerializationTypes.h
	Code/CryCommon/CryNetwork/SerializeFwd.h
	Code/CryCommon/CryPhysics/IPhysics.h
	Code/CryCommon/CryPhysics/physinterface.h
	Code/CryCommon/CryPhysics/primitives.h
	Code/CryCommon/CryRenderer/IRenderAuxGeom.h
	Code/CryCommon/CryRenderer/IRenderMesh.h
	Code/CryCommon/CryRenderer/IRenderer.h
	Code/CryCommon/CryRenderer/IShader.h
	Code/CryCommon/CryRenderer/ISkinnable.h
	Code/CryCommon/CryRenderer/IVideoPlayer.h
	Code/CryCommon/CryRenderer/RendElement.h
	Code/CryCommon/CryRenderer/VertexBufferSource.h
	Code/CryCommon/CryRenderer/VertexFormats.h
	Code/CryCommon/CryScriptSystem/IScriptSystem.h
	Code/CryCommon/CrySoundSystem/IMusicSystem.h
	Code/CryCommon/CrySoundSystem/IReverbManager.h
	Code/CryCommon/CrySoundSystem/ISound.h
	Code/CryCommon/CrySoundSystem/ISoundMoodManager.h
	Code/CryCommon/CrySystem/CryColorCode.h
	Code/CryCommon/CrySystem/CryFile.h
	Code/CryCommon/CrySystem/CryPath.h
	Code/CryCommon/CrySystem/CryVersion.h
	Code/CryCommon/CrySystem/FrameProfiler.h
	Code/CryCommon/CrySystem/IAVI_Reader.h
	Code/CryCommon/CrySystem/IBudgetingSystem.h
	Code/CryCommon/CrySystem/ICmdLine.h
	Code/CryCommon/CrySystem/IConsole.h
	Code/CryCommon/CrySystem/ICryPak.h
	Code/CryCommon/CrySystem/IDataProbe.h
	Code/CryCommon/CrySystem/IFlashPlayer.h
	Code/CryCommon/CrySystem/IGlobalTaskScheduler.h
	Code/CryCommon/CrySystem/IHardwareMouse.h
	Code/CryCommon/CrySystem/ILog.h
	Code/CryCommon/CrySystem/IProcess.h
	Code/CryCommon/CrySystem/IReadWriteXMLSink.h
	Code/CryCommon/CrySystem/IResourceCollector.h
	Code/CryCommon/CrySystem/IStreamEngine.h
	Code/CryCommon/CrySystem/ISystem.h
	Code/CryCommon/CrySystem/ITestSystem.h
	Code/CryCommon/CrySystem/ITextModeConsole.h
	Code/CryCommon/CrySystem/IThreadTask.h
	Code/CryCommon/CrySystem/ITimer.h
	Code/CryCommon/CrySystem/IValidator.h
	Code/CryCommon/CrySystem/IXml.h
	Code/CryCommon/CrySystem/TimeValue.h
	Code/CryCommon/CrySystem/gEnv.h
	Code/CryGame/Actors/Actor.cpp
	Code/CryGame/Actors/Actor.h
	Code/CryGame/Actors/Alien/Alien.cpp
	Code/CryGame/Actors/Alien/Alien.h
	Code/CryGame/Actors/Alien/CompatibilityAlienMovementController.cpp
	Code/CryGame/Actors/Alien/CompatibilityAlienMovementController.h
	Code/CryGame/Actors/Alien/Hunter.cpp
	Code/CryGame/Actors/Alien/Hunter.h
	Code/CryGame/Actors/Alien/Observer.cpp
	Code/CryGame/Actors/Alien/Observer.h
	Code/CryGame/Actors/Alien/Scout.cpp
	Code/CryGame/Actors/Alien/Scout.h
	Code/CryGame/Actors/Alien/Trooper.cpp
	Code/CryGame/Actors/Alien/Trooper.h
	Code/CryGame/Actors/GrabHandler.cpp
	Code/CryGame/Actors/GrabHandler.h
	Code/CryGame/Actors/Player/IPlayerInput.h
	Code/CryGame/Actors/Player/NanoSuit.cpp
	Code/CryGame/Actors/Player/NanoSuit.h
	Code/CryGame/Actors/Player/NetPlayerInput.cpp
	Code/CryGame/Actors/Player/NetPlayerInput.h
	Code/CryGame/Actors/Player/Player.cpp
	Code/CryGame/Actors/Player/Player.h
	Code/CryGame/Actors/Player/PlayerInput.cpp
	Code/CryGame/Actors/Player/PlayerInput.h
	Code/CryGame/Actors/Player/PlayerMovement.cpp
	Code/CryGame/Actors/Player/PlayerMovement.h
	Code/CryGame/Actors/Player/PlayerMovementController.cpp
	Code/CryGame/Actors/Player/PlayerMovementController.h
	Code/CryGame/Actors/Player/PlayerRotation.cpp
	Code/CryGame/Actors/Player/PlayerRotation.h
	Code/CryGame/Actors/Player/PlayerView.cpp
	Code/CryGame/Actors/Player/PlayerView.h
	Code/CryGame/Actors/Player/WeaponAttachmentManager.cpp
	Code/CryGame/Actors/Player/WeaponAttachmentManager.h
	Code/CryGame/Actors/ScreenEffects.cpp
	Code/CryGame/Actors/ScreenEffects.h
	Code/CryGame/Actors/ScriptBind_Actor.cpp
	Code/CryGame/Actors/ScriptBind_Actor.h
	Code/CryGame/Actors/Shark/Shark.cpp
	Code/CryGame/Actors/Shark/Shark.h
	Code/CryGame/Actors/Shark/SharkMovementController.cpp
	Code/CryGame/Actors/Shark/SharkMovementController.h
	Code/CryGame/BulletTime.cpp
	Code/CryGame/BulletTime.h
	Code/CryGame/ClientSynchedStorage.cpp
	Code/CryGame/ClientSynchedStorage.h
	Code/CryGame/Environment/BattleDust.cpp
	Code/CryGame/Environment/BattleDust.h
	Code/CryGame/Environment/Shake.cpp
	Code/CryGame/Environment/Shake.h
	Code/CryGame/Environment/Tornado/FlowTornado.cpp
	Code/CryGame/Environment/Tornado/FlowTornado.h
	Code/CryGame/Environment/Tornado/Tornado.cpp
	Code/CryGame/Environment/Tornado/Tornado.h
	Code/CryGame/FlashAnimation.cpp
	Code/CryGame/FlashAnimation.h
	Code/CryGame/Game.cpp
	Code/CryGame/Game.h
	Code/CryGame/GameActions.cpp
	Code/CryGame/GameActions.h
	Code/CryGame/GameCVars.cpp
	Code/CryGame/GameCVars.h
	Code/CryGame/GameFactory.cpp
	Code/CryGame/GameFactory.h
	Code/CryGame/GameRules.cpp
	Code/CryGame/GameRules.h
	Code/CryGame/GameRulesClientServer.cpp
	Code/CryGame/HUD/FlashPlayerNULL.h
	Code/CryGame/HUD/GameFlashAnimation.cpp
	Code/CryGame/HUD/GameFlashAnimation.h
	Code/CryGame/HUD/GameFlashLogic.cpp
	Code/CryGame/HUD/GameFlashLogic.h
	Code/CryGame/HUD/HUD.cpp
	Code/CryGame/HUD/HUD.h
	Code/CryGame/HUD/HUDAirStrike.cpp
	Code/CryGame/HUD/HUDCommon.cpp
	Code/CryGame/HUD/HUDCommon.h
	Code/CryGame/HUD/HUDCrosshair.cpp
	Code/CryGame/HUD/HUDCrosshair.h
	Code/CryGame/HUD/HUDEnums.h
	Code/CryGame/HUD/HUDHealthEnergyWeapon.cpp
	Code/CryGame/HUD/HUDInterfaceEffects.cpp
	Code/CryGame/HUD/HUDMissionObjectiveSystem.cpp
	Code/CryGame/HUD/HUDMissionObjectiveSystem.h
	Code/CryGame/HUD/HUDObituary.cpp
	Code/CryGame/HUD/HUDObituary.h
	Code/CryGame/HUD/HUDObject.cpp
	Code/CryGame/HUD/HUDObject.h
	Code/CryGame/HUD/HUDPDA.cpp
	Code/CryGame/HUD/HUDPowerStruggle.cpp
	Code/CryGame/HUD/HUDPowerStruggle.h
	Code/CryGame/HUD/HUDRadar.cpp
	Code/CryGame/HUD/HUDRadar.h
	Code/CryGame/HUD/HUDScopes.cpp
	Code/CryGame/HUD/HUDScopes.h
	Code/CryGame/HUD/HUDScore.cpp
	Code/CryGame/HUD/HUDScore.h
	Code/CryGame/HUD/HUDSilhouettes.cpp
	Code/CryGame/HUD/HUDSilhouettes.h
	Code/CryGame/HUD/HUDSoundImpl.cpp
	Code/CryGame/HUD/HUDTagNames.cpp
	Code/CryGame/HUD/HUDTagNames.h
	Code/CryGame/HUD/HUDTextArea.cpp
	Code/CryGame/HUD/HUDTextArea.h
	Code/CryGame/HUD/HUDTextChat.cpp
	Code/CryGame/HUD/HUDTextChat.h
	Code/CryGame/HUD/HUDTextEvents.cpp
	Code/CryGame/HUD/HUDVehicleInterface.cpp
	Code/CryGame/HUD/HUDVehicleInterface.h
	Code/CryGame/HUD/HUDWeaponAccessories.cpp
	Code/CryGame/HUD/ScriptBind_HUD.cpp
	Code/CryGame/HUD/ScriptBind_HUD.h
	Code/CryGame/Items/FlowItemAnimation.cpp
	Code/CryGame/Items/FlowItemAnimation.h
	Code/CryGame/Items/Item.cpp
	Code/CryGame/Items/Item.h
	Code/CryGame/Items/ItemAccessory.cpp
	Code/CryGame/Items/ItemClientServer.cpp
	Code/CryGame/Items/ItemDualWield.cpp
	Code/CryGame/Items/ItemEffect.cpp
	Code/CryGame/Items/ItemEvents.cpp
	Code/CryGame/Items/ItemParamReader.h
	Code/CryGame/Items/ItemParams.cpp
	Code/CryGame/Items/ItemResource.cpp
	Code/CryGame/Items/ItemScheduler.cpp
	Code/CryGame/Items/ItemScheduler.h
	Code/CryGame/Items/ItemSharedParams.cpp
	Code/CryGame/Items/ItemSharedParams.h
	Code/CryGame/Items/ItemString.cpp
	Code/CryGame/Items/ItemString.h
	Code/CryGame/Items/ItemView.cpp
	Code/CryGame/Items/Lam.cpp
	Code/CryGame/Items/Lam.h
	Code/CryGame/Items/PlayerFeature.cpp
	Code/CryGame/Items/PlayerFeature.h
	Code/CryGame/Items/ScriptBind_Item.cpp
	Code/CryGame/Items/ScriptBind_Item.h
	Code/CryGame/Items/Weapons/AmmoParams.cpp
	Code/CryGame/Items/Weapons/AmmoParams.h
	Code/CryGame/Items/Weapons/AmmoPickup.cpp
	Code/CryGame/Items/Weapons/AmmoPickup.h
	Code/CryGame/Items/Weapons/Binocular.cpp
	Code/CryGame/Items/Weapons/Binocular.h
	Code/CryGame/Items/Weapons/C4.cpp
	Code/CryGame/Items/Weapons/C4.h
	Code/CryGame/Items/Weapons/C4Detonator.cpp
	Code/CryGame/Items/Weapons/C4Detonator.h
	Code/CryGame/Items/Weapons/DebugGun.cpp
	Code/CryGame/Items/Weapons/DebugGun.h
	Code/CryGame/Items/Weapons/FireModes/Automatic.cpp
	Code/CryGame/Items/Weapons/FireModes/Automatic.h
	Code/CryGame/Items/Weapons/FireModes/Beam.cpp
	Code/CryGame/Items/Weapons/FireModes/Beam.h
	Code/CryGame/Items/Weapons/FireModes/Burst.cpp
	Code/CryGame/Items/Weapons/FireModes/Burst.h
	Code/CryGame/Items/Weapons/FireModes/Charge.cpp
	Code/CryGame/Items/Weapons/FireModes/Charge.h
	Code/CryGame/Items/Weapons/FireModes/Detonate.cpp
	Code/CryGame/Items/Weapons/FireModes/Detonate.h
	Code/CryGame/Items/Weapons/FireModes/FreezingBeam.cpp
	Code/CryGame/Items/Weapons/FireModes/FreezingBeam.h
	Code/CryGame/Items/Weapons/FireModes/Melee.cpp
	Code/CryGame/Items/Weapons/FireModes/Melee.h
	Code/CryGame/Items/Weapons/FireModes/Plant.cpp
	Code/CryGame/Items/Weapons/FireModes/Plant.h
	Code/CryGame/Items/Weapons/FireModes/Rapid.cpp
	Code/CryGame/Items/Weapons/FireModes/Rapid.h
	Code/CryGame/Items/Weapons/FireModes/Scan.cpp
	Code/CryGame/Items/Weapons/FireModes/Scan.h
	Code/CryGame/Items/Weapons/FireModes/Shotgun.cpp
	Code/CryGame/Items/Weapons/FireModes/Shotgun.h
	Code/CryGame/Items/Weapons/FireModes/Single.cpp
	Code/CryGame/Items/Weapons/FireModes/Single.h
	Code/CryGame/Items/Weapons/FireModes/SingleTG.cpp
	Code/CryGame/Items/Weapons/FireModes/SingleTG.h
	Code/CryGame/Items/Weapons/FireModes/Throw.cpp
	Code/CryGame/Items/Weapons/FireModes/Throw.h
	Code/CryGame/Items/Weapons/FireModes/WorkOnTarget.cpp
	Code/CryGame/Items/Weapons/FireModes/WorkOnTarget.h
	Code/CryGame/Items/Weapons/Fists.cpp
	Code/CryGame/Items/Weapons/Fists.h
	Code/CryGame/Items/Weapons/GunTurret.cpp
	Code/CryGame/Items/Weapons/GunTurret.h
	Code/CryGame/Items/Weapons/OffHand.cpp
	Code/CryGame/Items/Weapons/OffHand.h
	Code/CryGame/Items/Weapons/Projectile.cpp
	Code/CryGame/Items/Weapons/Projectile.h
	Code/CryGame/Items/Weapons/Projectiles/AVMine.cpp
	Code/CryGame/Items/Weapons/Projectiles/AVMine.h
	Code/CryGame/Items/Weapons/Projectiles/Bullet.cpp
	Code/CryGame/Items/Weapons/Projectiles/Bullet.h
	Code/CryGame/Items/Weapons/Projectiles/C4Projectile.cpp
	Code/CryGame/Items/Weapons/Projectiles/C4Projectile.h
	Code/CryGame/Items/Weapons/Projectiles/Claymore.cpp
	Code/CryGame/Items/Weapons/Projectiles/Claymore.h
	Code/CryGame/Items/Weapons/Projectiles/EMPField.cpp
	Code/CryGame/Items/Weapons/Projectiles/EMPField.h
	Code/CryGame/Items/Weapons/Projectiles/HomingMissile.cpp
	Code/CryGame/Items/Weapons/Projectiles/HomingMissile.h
	Code/CryGame/Items/Weapons/Projectiles/Rock.cpp
	Code/CryGame/Items/Weapons/Projectiles/Rock.h
	Code/CryGame/Items/Weapons/Projectiles/Rocket.cpp
	Code/CryGame/Items/Weapons/Projectiles/Rocket.h
	Code/CryGame/Items/Weapons/Projectiles/TacBullet.cpp
	Code/CryGame/Items/Weapons/Projectiles/TacBullet.h
	Code/CryGame/Items/Weapons/Projectiles/TagBullet.cpp
	Code/CryGame/Items/Weapons/Projectiles/TagBullet.h
	Code/CryGame/Items/Weapons/ReferenceWeapon.cpp
	Code/CryGame/Items/Weapons/ReferenceWeapon.h
	Code/CryGame/Items/Weapons/RocketLauncher.cpp
	Code/CryGame/Items/Weapons/RocketLauncher.h
	Code/CryGame/Items/Weapons/ScriptBind_Weapon.cpp
	Code/CryGame/Items/Weapons/ScriptBind_Weapon.h
	Code/CryGame/Items/Weapons/ThrowableWeapon.cpp
	Code/CryGame/Items/Weapons/ThrowableWeapon.h
	Code/CryGame/Items/Weapons/TracerManager.cpp
	Code/CryGame/Items/Weapons/TracerManager.h
	Code/CryGame/Items/Weapons/VehicleWeapon.cpp
	Code/CryGame/Items/Weapons/VehicleWeapon.h
	Code/CryGame/Items/Weapons/Weapon.cpp
	Code/CryGame/Items/Weapons/Weapon.h
	Code/CryGame/Items/Weapons/WeaponClientServer.cpp
	Code/CryGame/Items/Weapons/WeaponEvent.cpp
	Code/CryGame/Items/Weapons/WeaponInput.cpp
	Code/CryGame/Items/Weapons/WeaponSystem.cpp
	Code/CryGame/Items/Weapons/WeaponSystem.h
	Code/CryGame/Items/Weapons/ZoomModes/IronSight.cpp
	Code/CryGame/Items/Weapons/ZoomModes/IronSight.h
	Code/CryGame/Items/Weapons/ZoomModes/Scope.cpp
	Code/CryGame/Items/Weapons/ZoomModes/Scope.h
	Code/CryGame/MPTutorial.cpp
	Code/CryGame/MPTutorial.h
	Code/CryGame/Menus/CreateGame.cpp
	Code/CryGame/Menus/CreateGame.h
	Code/CryGame/Menus/FlashMenuObject.cpp
	Code/CryGame/Menus/FlashMenuObject.h
	Code/CryGame/Menus/FlashMenuObjectOptions.cpp
	Code/CryGame/Menus/FlashMenuObjectProfiles.cpp
	Code/CryGame/Menus/FlashMenuObjectSingleplayer.cpp
	Code/CryGame/Menus/FlashMenuScreen.cpp
	Code/CryGame/Menus/FlashMenuScreen.h
	Code/CryGame/Menus/GameNetworkProfile.cpp
	Code/CryGame/Menus/GameNetworkProfile.h
	Code/CryGame/Menus/MPHub.cpp
	Code/CryGame/Menus/MPHub.h
	Code/CryGame/Menus/MPLobbyUI.cpp
	Code/CryGame/Menus/MPLobbyUI.h
	Code/CryGame/Menus/MultiplayerMenu.cpp
	Code/CryGame/Menus/MultiplayerMenu.h
	Code/CryGame/Menus/OptionsManager.cpp
	Code/CryGame/Menus/OptionsManager.h
	Code/CryGame/Menus/QuickGame.cpp
	Code/CryGame/Menus/QuickGame.h
	Code/CryGame/NetInputChainDebug.cpp
	Code/CryGame/NetInputChainDebug.h
	Code/CryGame/Nodes/FlowActorSensor.cpp
	Code/CryGame/Nodes/FlowFadeNode.cpp
	Code/CryGame/Nodes/FlowHitInfoNode.cpp
	Code/CryGame/Nodes/FlowNanoSuitNodes.cpp
	Code/CryGame/Nodes/FlowPlayerStagingNode.cpp
	Code/CryGame/Nodes/FlowPostFXNodes.cpp
	Code/CryGame/Nodes/FlowVehicleNodes.cpp
	Code/CryGame/Nodes/G2FlowBaseNode.h
	Code/CryGame/Nodes/HUDNodes.cpp
	Code/CryGame/Nodes/MPNodes.cpp
	Code/CryGame/Nodes/WeaponNodes.cpp
	Code/CryGame/Radio.cpp
	Code/CryGame/Radio.h
	Code/CryGame/SPAnalyst.cpp
	Code/CryGame/SPAnalyst.h
	Code/CryGame/ScriptBind_Game.cpp
	Code/CryGame/ScriptBind_Game.h
	Code/CryGame/ScriptBind_GameRules.cpp
	Code/CryGame/ScriptBind_GameRules.h
	Code/CryGame/ScriptControlledPhysics.cpp
	Code/CryGame/ScriptControlledPhysics.h
	Code/CryGame/ScriptUtils.cpp
	Code/CryGame/ScriptUtils.h
	Code/CryGame/ServerSynchedStorage.cpp
	Code/CryGame/ServerSynchedStorage.h
	Code/CryGame/ShotValidator.cpp
	Code/CryGame/ShotValidator.h
	Code/CryGame/SoundMoods.cpp
	Code/CryGame/SoundMoods.h
	Code/CryGame/SynchedStorage.cpp
	Code/CryGame/SynchedStorage.h
	Code/CryGame/Vehicles/Action/VehicleActionAutomaticDoor.cpp
	Code/CryGame/Vehicles/Action/VehicleActionAutomaticDoor.h
	Code/CryGame/Vehicles/Action/VehicleActionDeployRope.cpp
	Code/CryGame/Vehicles/Action/VehicleActionDeployRope.h
	Code/CryGame/Vehicles/Action/VehicleActionEntityAttachment.cpp
	Code/CryGame/Vehicles/Action/VehicleActionEntityAttachment.h
	Code/CryGame/Vehicles/Action/VehicleActionLandingGears.cpp
	Code/CryGame/Vehicles/Action/VehicleActionLandingGears.h
	Code/CryGame/Vehicles/Damage/VehicleDamageBehaviorBurn.cpp
	Code/CryGame/Vehicles/Damage/VehicleDamageBehaviorBurn.h
	Code/CryGame/Vehicles/Damage/VehicleDamageBehaviorCameraShake.cpp
	Code/CryGame/Vehicles/Damage/VehicleDamageBehaviorCameraShake.h
	Code/CryGame/Vehicles/Damage/VehicleDamageBehaviorCollisionEx.cpp
	Code/CryGame/Vehicles/Damage/VehicleDamageBehaviorCollisionEx.h
	Code/CryGame/Vehicles/Damage/VehicleDamageBehaviorExplosion.cpp
	Code/CryGame/Vehicles/Damage/VehicleDamageBehaviorExplosion.h
	Code/CryGame/Vehicles/Damage/VehicleDamageBehaviorTire.cpp
	Code/CryGame/Vehicles/Damage/VehicleDamageBehaviorTire.h
	Code/CryGame/Vehicles/Movement/VehicleMovementAmphibious.cpp
	Code/CryGame/Vehicles/Movement/VehicleMovementAmphibious.h
	Code/CryGame/Vehicles/Movement/VehicleMovementBase.cpp
	Code/CryGame/Vehicles/Movement/VehicleMovementBase.h
	Code/CryGame/Vehicles/Movement/VehicleMovementHelicopter.cpp
	Code/CryGame/Vehicles/Movement/VehicleMovementHelicopter.h
	Code/CryGame/Vehicles/Movement/VehicleMovementHovercraft.cpp
	Code/CryGame/Vehicles/Movement/VehicleMovementHovercraft.h
	Code/CryGame/Vehicles/Movement/VehicleMovementStdBoat.cpp
	Code/CryGame/Vehicles/Movement/VehicleMovementStdBoat.h
	Code/CryGame/Vehicles/Movement/VehicleMovementStdWheeled.cpp
	Code/CryGame/Vehicles/Movement/VehicleMovementStdWheeled.h
	Code/CryGame/Vehicles/Movement/VehicleMovementTank.cpp
	Code/CryGame/Vehicles/Movement/VehicleMovementTank.h
	Code/CryGame/Vehicles/Movement/VehicleMovementTweaks.cpp
	Code/CryGame/Vehicles/Movement/VehicleMovementTweaks.h
	Code/CryGame/Vehicles/Movement/VehicleMovementVTOL.cpp
	Code/CryGame/Vehicles/Movement/VehicleMovementVTOL.h
	Code/CryGame/Vehicles/Movement/VehicleMovementWarrior.cpp
	Code/CryGame/Vehicles/Movement/VehicleMovementWarrior.h
	Code/CryGame/Vehicles/VehicleClient.cpp
	Code/CryGame/Vehicles/VehicleClient.h
	Code/CryGame/Voting.cpp
	Code/CryGame/Voting.h
	Code/CryMP/Client/Client.cpp
	Code/CryMP/Client/Client.h
	Code/CryMP/Client/DrawTools.cpp
	Code/CryMP/Client/DrawTools.h
	Code/CryMP/Client/EngineCache.cpp
	Code/CryMP/Client/EngineCache.h
	Code/CryMP/Client/FFontHooks.cpp
	Code/CryMP/Client/FFontHooks.h
	Code/CryMP/Client/FileCache.cpp
	Code/CryMP/Client/FileCache.h
	Code/CryMP/Client/FileDownloader.cpp
	Code/CryMP/Client/FileDownloader.h
	Code/CryMP/Client/FileRedirector.cpp
	Code/CryMP/Client/FileRedirector.h
	Code/CryMP/Client/FlashFileHooks.cpp
	Code/CryMP/Client/FlashFileHooks.h
	Code/CryMP/Client/MapDownloader.cpp
	Code/CryMP/Client/MapDownloader.h
	Code/CryMP/Client/MapExtractor.cpp
	Code/CryMP/Client/MapExtractor.h
	Code/CryMP/Client/ParticleManager.cpp
	Code/CryMP/Client/ParticleManager.h
	Code/CryMP/Client/ScriptBind_CPPAPI.cpp
	Code/CryMP/Client/ScriptBind_CPPAPI.h
	Code/CryMP/Client/ScriptCallbacks.cpp
	Code/CryMP/Client/ScriptCallbacks.h
	Code/CryMP/Client/ScriptCommands.cpp
	Code/CryMP/Client/ScriptCommands.h
	Code/CryMP/Client/ServerBrowser.cpp
	Code/CryMP/Client/ServerBrowser.h
	Code/CryMP/Client/ServerConnector.cpp
	Code/CryMP/Client/ServerConnector.h
	Code/CryMP/Client/ServerInfo.cpp
	Code/CryMP/Client/ServerInfo.h
	Code/CryMP/Client/ServerPAK.cpp
	Code/CryMP/Client/ServerPAK.h
	Code/CryMP/Client/SpeedAggregator.cpp
	Code/CryMP/Client/SpeedAggregator.h
	Code/CryMP/Common/Executor.cpp
	Code/CryMP/Common/Executor.h
	Code/CryMP/Common/GSMasterHook.cpp
	Code/CryMP/Common/GSMasterHook.h
	Code/CryMP/Common/HTTP.cpp
	Code/CryMP/Common/HTTP.h
	Code/CryMP/Common/HTTPClient.cpp
	Code/CryMP/Common/HTTPClient.h
	Code/CryMP/Server/Server.cpp
	Code/CryMP/Server/Server.h
	Code/CryScriptSystem/LuaLibs/bitlib.c
	Code/CryScriptSystem/ScriptBindings/ScriptBindings.cpp
	Code/CryScriptSystem/ScriptBindings/ScriptBindings.h
	Code/CryScriptSystem/ScriptBindings/ScriptBind_Movie.cpp
	Code/CryScriptSystem/ScriptBindings/ScriptBind_Movie.h
	Code/CryScriptSystem/ScriptBindings/ScriptBind_Particle.cpp
	Code/CryScriptSystem/ScriptBindings/ScriptBind_Particle.h
	Code/CryScriptSystem/ScriptBindings/ScriptBind_Physics.cpp
	Code/CryScriptSystem/ScriptBindings/ScriptBind_Physics.h
	Code/CryScriptSystem/ScriptBindings/ScriptBind_Script.cpp
	Code/CryScriptSystem/ScriptBindings/ScriptBind_Script.h
	Code/CryScriptSystem/ScriptBindings/ScriptBind_Sound.cpp
	Code/CryScriptSystem/ScriptBindings/ScriptBind_Sound.h
	Code/CryScriptSystem/ScriptBindings/ScriptBind_System.cpp
	Code/CryScriptSystem/ScriptBindings/ScriptBind_System.h
	Code/CryScriptSystem/FunctionHandler.cpp
	Code/CryScriptSystem/FunctionHandler.h
	Code/CryScriptSystem/ScriptSystem.cpp
	Code/CryScriptSystem/ScriptSystem.h
	Code/CryScriptSystem/ScriptTable.cpp
	Code/CryScriptSystem/ScriptTable.h
	Code/CryScriptSystem/ScriptTimerManager.cpp
	Code/CryScriptSystem/ScriptTimerManager.h
	Code/CryScriptSystem/ScriptUtil.cpp
	Code/CryScriptSystem/ScriptUtil.h
	Code/CrySystem/CPUInfo.cpp
	Code/CrySystem/CPUInfo.h
	Code/CrySystem/CryLog.cpp
	Code/CrySystem/CryLog.h
	Code/CrySystem/CryMemoryManager.cpp
	Code/CrySystem/CryMemoryManager.h
	Code/CrySystem/GameWindow.cpp
	Code/CrySystem/GameWindow.h
	Code/CrySystem/HardwareMouse.cpp
	Code/CrySystem/HardwareMouse.h
	Code/CrySystem/LocalizationManager.cpp
	Code/CrySystem/LocalizationManager.h
	Code/CrySystem/Logger.cpp
	Code/CrySystem/Logger.h
	Code/CrySystem/RandomGenerator.cpp
	Code/CrySystem/RandomGenerator.h
	Code/Launcher/Launcher.cpp
	Code/Launcher/Launcher.h
	Code/Launcher/Main.cpp
	Code/Launcher/MemoryPatch.cpp
	Code/Launcher/MemoryPatch.h
	Code/Launcher/Resources.h
	Code/Library/CPUID.cpp
	Code/Library/CPUID.h
	Code/Library/CrashLogger.cpp
	Code/Library/CrashLogger.h
	Code/Library/StringTools.cpp
	Code/Library/StringTools.h
	Code/Library/Util.cpp
	Code/Library/Util.h
	Code/Library/WinAPI.cpp
	Code/Library/WinAPI.h
	Resources/Launcher.manifest
	Resources/Launcher.rc
	ThirdParty/Lua/src/lapi.c
	ThirdParty/Lua/src/lapi.h
	ThirdParty/Lua/src/lauxlib.c
	ThirdParty/Lua/src/lauxlib.h
	ThirdParty/Lua/src/lbaselib.c
	ThirdParty/Lua/src/lcode.c
	ThirdParty/Lua/src/lcode.h
	ThirdParty/Lua/src/ldblib.c
	ThirdParty/Lua/src/ldebug.c
	ThirdParty/Lua/src/ldebug.h
	ThirdParty/Lua/src/ldo.c
	ThirdParty/Lua/src/ldo.h
	ThirdParty/Lua/src/ldump.c
	ThirdParty/Lua/src/lfunc.c
	ThirdParty/Lua/src/lfunc.h
	ThirdParty/Lua/src/lgc.c
	ThirdParty/Lua/src/lgc.h
	ThirdParty/Lua/src/linit.c
	ThirdParty/Lua/src/liolib.c
	ThirdParty/Lua/src/llex.c
	ThirdParty/Lua/src/llex.h
	ThirdParty/Lua/src/llimits.h
	ThirdParty/Lua/src/lmathlib.c
	ThirdParty/Lua/src/lmem.c
	ThirdParty/Lua/src/lmem.h
	ThirdParty/Lua/src/loadlib.c
	ThirdParty/Lua/src/lobject.c
	ThirdParty/Lua/src/lobject.h
	ThirdParty/Lua/src/lopcodes.c
	ThirdParty/Lua/src/lopcodes.h
	ThirdParty/Lua/src/loslib.c
	ThirdParty/Lua/src/lparser.c
	ThirdParty/Lua/src/lparser.h
	ThirdParty/Lua/src/lstate.c
	ThirdParty/Lua/src/lstate.h
	ThirdParty/Lua/src/lstring.c
	ThirdParty/Lua/src/lstring.h
	ThirdParty/Lua/src/lstrlib.c
	ThirdParty/Lua/src/ltable.c
	ThirdParty/Lua/src/ltable.h
	ThirdParty/Lua/src/ltablib.c
	ThirdParty/Lua/src/ltm.c
	ThirdParty/Lua/src/ltm.h
	ThirdParty/Lua/src/luaconf.h
	ThirdParty/Lua/src/lua.h
	ThirdParty/Lua/src/lualib.h
	ThirdParty/Lua/src/lundump.c
	ThirdParty/Lua/src/lundump.h
	ThirdParty/Lua/src/lvm.c
	ThirdParty/Lua/src/lvm.h
	ThirdParty/Lua/src/lzio.c
	ThirdParty/Lua/src/lzio.h
	ThirdParty/miniz/miniz.c
	ThirdParty/miniz/miniz.h
	ThirdParty/nlohmann/json.hpp
	ThirdParty/nlohmann/json_fwd.hpp
	ThirdParty/picosha2.h
	ThirdParty/tracy/public/TracyClient.cpp
	${PROJECT_BINARY_DIR}/config.h
	${PROJECT_BINARY_DIR}/version.h
 "Code/CryMP/Server/ServerPublisher.cpp" "Code/CryMP/Server/ServerPublisher.h" "Code/CryMP/Server/ServerLog.h" "Code/CryMP/Server/ServerEvents.h" "Code/CryMP/Server/ServerEvents.cpp" "Code/CryMP/Server/ScriptBind_Server.cpp" "Code/CryMP/Server/ScriptBind_Server.h" "Code/CryMP/Server/LuaFileSystem.cpp" "Code/CryMP/Server/LuaFileSystem.h" "Code/CryMP/Server/ServerUtils.h" "Code/CryMP/Server/ServerCVars.h" "Code/CryMP/Server/ServerCVars.cpp" "Code/CryMP/Server/ServerStats.h"  "Code/CryMP/Server/ServerCVarCallbacks.h" "Code/CryMP/Server/QuitHook.h" "Code/CryMP/Server/QuitHook.cpp" "Code/CryMP/Server/ServerTimer.h")

################################################################################

if(CRYMP_USE_MIMALLOC)
	add_library(mimalloc STATIC
		ThirdParty/mimalloc/src/alloc.c
		ThirdParty/mimalloc/src/alloc-aligned.c
		ThirdParty/mimalloc/src/alloc-posix.c
		ThirdParty/mimalloc/src/arena.c
		ThirdParty/mimalloc/src/bitmap.c
		ThirdParty/mimalloc/src/heap.c
		ThirdParty/mimalloc/src/init.c
		ThirdParty/mimalloc/src/options.c
		ThirdParty/mimalloc/src/os.c
		ThirdParty/mimalloc/src/page.c
		ThirdParty/mimalloc/src/random.c
		ThirdParty/mimalloc/src/segment.c
		ThirdParty/mimalloc/src/segment-map.c
		ThirdParty/mimalloc/src/stats.c
		ThirdParty/mimalloc/src/prim/prim.c
	)

	set_property(TARGET mimalloc PROPERTY C_STANDARD 11)
	target_include_directories(mimalloc SYSTEM PUBLIC ThirdParty/mimalloc/include)

	target_link_libraries(${CRYMP_SERVER_EXE} PRIVATE mimalloc)
endif()

################################################################################

set(CRYMP_SERVER_RESOURCES
	${PROJECT_SOURCE_DIR}/Resources/CursorAmber.cur
	${PROJECT_SOURCE_DIR}/Resources/CursorBlue.cur
	${PROJECT_SOURCE_DIR}/Resources/CursorGreen.cur
	${PROJECT_SOURCE_DIR}/Resources/CursorRed.cur
	${PROJECT_SOURCE_DIR}/Resources/CursorWhite.cur
	${PROJECT_SOURCE_DIR}/Resources/Game/Libs/UI/HUD_ChatSystem.gfx
	${PROJECT_SOURCE_DIR}/Resources/Game/Libs/UI/HUD_VehicleStats.gfx
	${PROJECT_SOURCE_DIR}/Resources/Game/Libs/UI/HUD_PDA_Buy.gfx
	${PROJECT_SOURCE_DIR}/Resources/Game/Libs/UI/Menus_Loading_MP.gfx
	${PROJECT_SOURCE_DIR}/Resources/Game/Libs/UI/HUD_HitIndicator.gfx
	${PROJECT_SOURCE_DIR}/Resources/Game/Libs/UI/HUD_Spectate.gfx
	${PROJECT_SOURCE_DIR}/Resources/Game/Libs/UI/HUD_MP_Radio_Buttons.gfx
	${PROJECT_SOURCE_DIR}/Resources/Game/Libs/UI/HUD_ChatSystem_HR.gfx
	${PROJECT_SOURCE_DIR}/Resources/Game/Libs/UI/HUD_KillLog.gfx
	${PROJECT_SOURCE_DIR}/Resources/Icon.ico
	${PROJECT_SOURCE_DIR}/masters.txt
)

set_source_files_properties(Resources/Launcher.rc PROPERTIES OBJECT_DEPENDS "${CRYMP_SERVER_RESOURCES}")

################################################################################

set_property(TARGET ${CRYMP_SERVER_EXE} PROPERTY CXX_STANDARD 20)
set_property(TARGET ${CRYMP_SERVER_EXE} PROPERTY CXX_STANDARD_REQUIRED ON)

if(BUILD_BITS EQUAL 64)
	target_compile_definitions(${CRYMP_SERVER_EXE} PUBLIC BUILD_64BIT)
endif()

if(CRYMP_TRACY_ENABLED)
	target_compile_definitions(${CRYMP_SERVER_EXE} PRIVATE TRACY_ENABLE)
endif()

if(CRYMP_CONSOLE_APP)
	set_target_properties(${CRYMP_SERVER_EXE} PROPERTIES WIN32_EXECUTABLE FALSE)
endif()

target_compile_definitions(${CRYMP_SERVER_EXE} PRIVATE _CRT_SECURE_NO_WARNINGS)

target_compile_options(${CRYMP_SERVER_EXE} PRIVATE /W3)

target_include_directories(${CRYMP_SERVER_EXE} PRIVATE Code ${PROJECT_BINARY_DIR})
target_include_directories(${CRYMP_SERVER_EXE} SYSTEM PRIVATE
	ThirdParty
	ThirdParty/Lua/src
	ThirdParty/miniz
	ThirdParty/tracy/public
)

target_link_libraries(${CRYMP_SERVER_EXE} PRIVATE dbghelp ws2_32 winhttp)

# prevent modern MSVC from enabling ASLR and unlock memory above 2 GB
target_link_options(${CRYMP_SERVER_EXE} PRIVATE /DYNAMICBASE:NO /LARGEADDRESSAWARE)

################################################################################

configure_file(config.h.in ${PROJECT_BINARY_DIR}/config.h)
