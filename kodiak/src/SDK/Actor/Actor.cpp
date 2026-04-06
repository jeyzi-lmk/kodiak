int16_t Actor::getHurtTime() {
    return Mem::read_value<int16_t>(Game::handle, reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this) + 0x19C));
}

ActorType Actor::getType() {
    return Mem::read_value<ActorType>(Game::handle, reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this) + 0x210));
}

bool Actor::hasType(ActorType type) {
    return (static_cast<int>(getType()) & static_cast<int>(type)) != 0;
}

std::optional<StateVectorComponent> Actor::getStateVector() {
    auto pointer = Mem::read_value<uintptr_t>(Game::handle, reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this) + 0x218));
    if(!pointer) return std::nullopt;

    return Mem::read_value<StateVectorComponent>(Game::handle, reinterpret_cast<void*>(pointer));
}

std::optional<AABBShapeComponent> Actor::getAABB() {
    auto pointer = Mem::read_value<uintptr_t>(Game::handle, reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this) + 0x220));
    if(!pointer) return std::nullopt;
    return Mem::read_value<AABBShapeComponent>(Game::handle, reinterpret_cast<void*>(pointer));
}

std::optional<ActorRotationComponent> Actor::getActorRotation() {
    auto pointer = Mem::read_value<uintptr_t>(Game::handle, reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this) + 0x228));
    if(!pointer) return std::nullopt;
    return Mem::read_value<ActorRotationComponent>(Game::handle, reinterpret_cast<void*>(pointer));
}

std::optional<Vec3<float>> Actor::getPosition() {
    auto stateVector = getStateVector();
    if(!stateVector) return std::nullopt;

    auto pos = stateVector->position;;
    return hasType(ActorType::Player) ? pos.sub(0, 1.62f, 0) : pos;
}

void Actor::setRotation(Vec2<float> rot) {
    auto pointer = Mem::read_value<uintptr_t>(Game::handle, reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this) + 0x228));
    if(!pointer) return;

    Mem::write_value<Vec2<float>>(Game::handle, reinterpret_cast<void*>(pointer), rot);
}

float Actor::getYaw() {
    auto rotation = getActorRotation();
    if(!rotation) return 0.0f;
    return rotation->rotation.y;
}

void Actor::setYaw(float yaw) {
    auto pointer = Mem::read_value<uintptr_t>(Game::handle, reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this) + 0x228));
    if(!pointer) return;

    Mem::write_value<float>(Game::handle, reinterpret_cast<void*>(pointer + 0x4), yaw);
}

float Actor::getPitch() {
    auto rotation = getActorRotation();
    if(!rotation) return 0.0f;
    return rotation->rotation.x;
}

void Actor::setPitch(float pitch) {
    auto pointer = Mem::read_value<uintptr_t>(Game::handle, reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this) + 0x228));
    if(!pointer) return;

    Mem::write_value<float>(Game::handle, reinterpret_cast<void*>(pointer + 0x0), pitch);
}

bool Actor::valid() {
    if(!hasType(ActorType::Mob) && !hasType(ActorType::Animal) && !hasType(ActorType::Humanoid) && !hasType(ActorType::Player) && !hasType(ActorType::Monster))
        return false;

    auto component = getAABB();
    if(!component)
        return false;

    if(component->size.x <= 0.01f || component->size.y <= 0.01f)
        return false;

    auto hurtTime = getHurtTime();
    if(hurtTime < 0 || hurtTime > 10)
        return false;

    return true;
}