#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <map>
#include <memory>
#include <mutex>
#include <vector>
#include <chrono>
#include <thread>
#include <tchar.h>
#include <iostream>
#include <Windows.h>
#include <tlhelp32.h>
#include <filesystem>
#include <string>
#include <shlObj.h>
#include <psapi.h>
#include <sstream>
#include <span>
#include <cstring>
#include <fstream>
#include <future>
#include <random>
#include <appmodel.h>
#include <comdef.h>
#include <algorithm>
#include <cctype>
#include <propsys.h>
#include <propkey.h>
#include <propvarutil.h>
#include <shlwapi.h>
#include <cstdint>
#include <optional>
#include <cmath>
#include <variant>
#include <cinttypes>
#include <deque>
#include <iomanip>
#include <ctime>
#include <functional>
#include <unordered_map>

#include "Util/Console.hpp"
#include "Util/NT.hpp"
#include "Util/Pattern.hpp"
#include "Util/Asm.hpp"
#include "Util/Mem.hpp"
#include "Util/Detour.hpp"
#include "Util/Vector.h"
#include "Util/Angle.hpp"
#include "Util/Command.hpp"

#include "Game/Hook/Hook.hpp"
#include "Game/Hook/HookRegistry.hpp"

#include "Game/Hook/Impl/LocalPlayerHook.hpp"
#include "Game/Hook/Impl/HitResultActorHook.hpp"
#include "Game/Hook/Impl/CameraHook.hpp"
#include "Game/Hook/Impl/MouseProcHook.hpp"

#include "SDK/Actor/Components/StateVectorComponent.hpp"
#include "SDK/Actor/Components/AABBShapeComponent.hpp"
#include "SDK/Actor/Components/ActorRotationComponent.hpp"
#include "SDK/Actor/ActorType.hpp"
#include "SDK/Actor/Camera.h"
#include "SDK/Actor/Actor.hpp"
#include "SDK/SDK.hpp"

#include "Game/Features/Feature.hpp"
#include "Game/Features/FeatureRegistry.hpp"
#include "Game/Features/Impl/AimAssist.hpp"

#include "Core/Diagnostics.hpp"
#include "Core/Versioning.hpp"
#include "Core/ConfigManager.hpp"

#include "Game/Game.hpp"
#include "Game/Runtime.hpp"
