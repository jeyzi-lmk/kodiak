#pragma once

class Actor {
public:
    int16_t getHurtTime();
    ActorType getType();
    bool hasType(ActorType type);
    std::optional<StateVectorComponent> getStateVector();
    std::optional<AABBShapeComponent> getAABB();
    std::optional<ActorRotationComponent> getActorRotation();
    std::optional<Vec3<float>> getPosition();
    void setRotation(Vec2<float>);
    float getYaw();
    void setYaw(float);
    float getPitch();
    void setPitch(float);
    bool valid();
};