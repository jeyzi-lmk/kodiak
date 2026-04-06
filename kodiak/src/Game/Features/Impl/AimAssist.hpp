#pragma once

class AimAssist final : public Feature {
public:
    bool onlyInGame = true;
    bool onlyWhileClicking = true;
    float maxAngle = 90.0f;
    float distance = 5.0f;

    float hSpeed = 5.0f;

    bool vertically = false;
    float vSpeed = 5.0f;

    AimAssist() : Feature("Aim Assist", "Automatically aim to target") {}

    __forceinline void tick(int i) {
        if(!isEnabled()) return;
        if(onlyInGame && !Game::isInGameScreen()) return;
        if(onlyWhileClicking && !Game::gMouseDown.load(std::memory_order_acquire)) return;

        auto player = SDK::getLocalPlayer();
        if(!player) return;
        if(!player->valid()) return;

        auto target = SDK::getHitResultActor();
        if(!target) return;
        if(!target->valid()) return;

        auto camera = SDK::getCamera();
        if(!camera) return;

        auto pPos = player->getPosition();
        if(!pPos.has_value()) return;

        auto tPos = target->getPosition();
        if(!tPos.has_value()) return;


        auto dist = pPos->dist(tPos.value());
        if(dist > distance || dist < 0.5f) return;

        auto angles = Angle::computeAim(pPos.value(), tPos.value());

        if(std::isnan(angles.x) || std::isinf(angles.x) || std::abs(angles.x) > 360.0f) {
#ifdef DEBUG
            Console::print("Invalid Yaw Angle: " + std::to_string(angles.x));
#endif
            return;
        }

        if(std::isnan(angles.y) || std::isinf(angles.y) || std::abs(angles.y) > 360.0f) {
#ifdef DEBUG
            Console::print("Invalid Pitch Angle: " + std::to_string(angles.y));
#endif
            return;
        }

        float yawDelta = Angle::normalizeYaw(angles.x - camera->yaw);
        float pitchDelta = angles.y - camera->pitch;

        float maxAngleRad = maxAngle * (Angle::PI / 180.f);

        if(std::abs(yawDelta) > maxAngleRad) return;
        if(std::abs(pitchDelta) > maxAngleRad) return;

        constexpr float pitchLim = Angle::PI * 0.5f - 0.01f;

        float targetYaw   = Angle::wrapPi(camera->yaw + yawDelta);
        float targetPitch = std::clamp(camera->pitch + pitchDelta, -pitchLim, pitchLim);

        float tYaw   = std::clamp(hSpeed / 400.f, 0.f, 1.f);
        float tPitch = std::clamp(vSpeed / 400.f, 0.f, 1.f);

        float newYaw   = Angle::lerpAngleRad(camera->yaw,   targetYaw,   tYaw);
        float newPitch = Angle::lerpAngleRad(camera->pitch, targetPitch, tPitch);

        SDK::setYaw(newYaw);
        if(vertically) SDK::setPitch(newPitch);
    }
};
