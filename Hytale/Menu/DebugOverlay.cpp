/*
 * Copyright (c) FishPlusPlus.
 */
#include "DebugOverlay.h"

#include <cfloat>   // FLT_MAX

#include "../Core.h"
#include "../Util/Util.h"
#include "../Renderer/FontRenderer/Fonts.h"
#include "../Features/FeatureHandler.h"
#include "../Features/ActualFeatures/LookAtInfo.h"

// ── types ─────────────────────────────────────────────────────────────────────

struct LookedAtBlock {
    bool        found   = false;
    int         x = 0, y = 0, z = 0;
    int         blockId = 0;
    std::string name;
};

struct LookedAtEntity {
    bool        found     = false;
    int         networkId = 0;
    std::string name;
    Vector3     position  = { 0.0f, 0.0f, 0.0f };
};

// ── helpers ───────────────────────────────────────────────────────────────────

static Vector3 GetLookForward() {
    GameInstance* gi = Util::getGameInstance();
    if (!gi || !gi->CameraModule || !gi->CameraModule->Controller)
        return Vector3(0.0f, 0.0f, -1.0f);

    float pitch, yaw;
    if (gi->CameraModule->thirdPerson) {
        Vector3 rot = gi->CameraModule->Controller->ThirdPersonRot;
        pitch =  rot.x;
        yaw   = -rot.y;
    } else {
        pitch =  Util::getLocalPlayer()->pitchRad;
        yaw   = -Util::getLocalPlayer()->yawRad;
    }

    return Vector3(
        cosf(pitch) * sinf(yaw),
        sinf(pitch),
        -cosf(pitch) * cosf(yaw)
    );
}

// Amanatides-Woo DDA — visits every voxel the ray passes through in order,
// guaranteed no skips regardless of direction.
static LookedAtBlock GetLookedAtBlock(float maxRange, const Vector3& origin, const Vector3& dir) {
    LookedAtBlock result;
    GameInstance* gi = Util::getGameInstance();
    if (!gi || !gi->MapModule) return result;

    int x = (int)floorf(origin.x);
    int y = (int)floorf(origin.y);
    int z = (int)floorf(origin.z);

    // Which direction we step on each axis; 0 if ray is axis-parallel
    const int stepX = dir.x > 0 ? 1 : (dir.x < 0 ? -1 : 0);
    const int stepY = dir.y > 0 ? 1 : (dir.y < 0 ? -1 : 0);
    const int stepZ = dir.z > 0 ? 1 : (dir.z < 0 ? -1 : 0);

    // Distance along ray to first voxel boundary on each axis
    const float tDeltaX = dir.x != 0.0f ? 1.0f / fabsf(dir.x) : FLT_MAX;
    const float tDeltaY = dir.y != 0.0f ? 1.0f / fabsf(dir.y) : FLT_MAX;
    const float tDeltaZ = dir.z != 0.0f ? 1.0f / fabsf(dir.z) : FLT_MAX;

    float tMaxX = dir.x != 0.0f ? ((dir.x > 0 ? (x + 1.0f - origin.x) : (origin.x - x)) / fabsf(dir.x)) : FLT_MAX;
    float tMaxY = dir.y != 0.0f ? ((dir.y > 0 ? (y + 1.0f - origin.y) : (origin.y - y)) / fabsf(dir.y)) : FLT_MAX;
    float tMaxZ = dir.z != 0.0f ? ((dir.z > 0 ? (z + 1.0f - origin.z) : (origin.z - z)) / fabsf(dir.z)) : FLT_MAX;

    float t = 0.0f;
    while (t <= maxRange) {
        int id = gi->MapModule->GetBlockID(x, y, z, 0);
        if (id > 1) {   // 0 = unloaded chunk, 1 = air
            result.found   = true;
            result.x       = x;
            result.y       = y;
            result.z       = z;
            result.blockId = id;
            ClientBlockType* bt = gi->MapModule->ClientBlockTypes->get(id);
            if (bt && Util::IsValidPtr(bt->Name))
                result.name = bt->Name->getString();
            else
                result.name = "unknown";
            break;
        }

        if (tMaxX < tMaxY && tMaxX < tMaxZ) {
            t = tMaxX; x += stepX; tMaxX += tDeltaX;
        } else if (tMaxY < tMaxZ) {
            t = tMaxY; y += stepY; tMaxY += tDeltaY;
        } else {
            t = tMaxZ; z += stepZ; tMaxZ += tDeltaZ;
        }
    }
    return result;
}

// Slab method ray-AABB intersection. Returns true and sets tHit to distance along ray.
static bool RayIntersectsAABB(const Vector3& orig, const Vector3& dir,
                               const Vector3& boxMin, const Vector3& boxMax,
                               float& tHit) {
    float tMin = 0.0f;
    float tMax = FLT_MAX;

    const float* o    = &orig.x;
    const float* d    = &dir.x;
    const float* bMin = &boxMin.x;
    const float* bMax = &boxMax.x;

    for (int i = 0; i < 3; i++) {
        if (fabsf(d[i]) < 1e-6f) {
            if (o[i] < bMin[i] || o[i] > bMax[i]) return false;
        } else {
            float t1 = (bMin[i] - o[i]) / d[i];
            float t2 = (bMax[i] - o[i]) / d[i];
            if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; }
            if (t1 > tMin) tMin = t1;
            if (t2 < tMax) tMax = t2;
            if (tMin > tMax) return false;
        }
    }

    tHit = tMin;
    return tMin >= 0.0f;
}

// Ray-AABB test against the same hitbox ESP draws (RenderPos + Hitbox.min/max).
// Returns the closest hit entity along the look ray, or found=false.
static LookedAtEntity GetLookedAtEntity(const Vector3& origin, const Vector3& forward) {
    LookedAtEntity result;
    float bestDist = FLT_MAX;

    SDK::global_mutex.lock();
    std::vector<EntityData> entities = SDK::entities;
    SDK::global_mutex.unlock();

    for (const auto& ent : entities) {
        if (ent.isLocalPlayer) continue;
        if (ent.entityType != Entity::EntityType::Character) continue;
        ValidPtrLoop(ent.entityPtr);   // guard against dangling pointer

        const BoundingBox& hb = ent.entityPtr->Hitbox;
        const Vector3 boxMin  = ent.entityPtr->RenderPos + hb.min;
        const Vector3 boxMax  = ent.entityPtr->RenderPos + hb.max;

        float tHit = 0.0f;
        if (!RayIntersectsAABB(origin, forward, boxMin, boxMax, tHit)) continue;
        if (tHit >= bestDist) continue;

        bestDist         = tHit;
        result.found     = true;
        result.networkId = ent.networkID;
        result.name      = ent.name;
        result.position  = ent.position;
    }
    return result;
}

// ── cache ─────────────────────────────────────────────────────────────────────
// Results are refreshed at most 3 times per second to avoid per-frame overhead.
// s_lastUpdate = 0 forces an immediate refresh on the next call (used on feature re-enable).

static constexpr double kCacheInterval = 1.0 / 3.0;

static LookedAtBlock  s_cachedBlk;
static LookedAtEntity s_cachedEnt;
static double         s_lastUpdate = 0.0;

static void UpdateCache(float range) {
    double now = Util::GetTime();
    if (now - s_lastUpdate < kCacheInterval)
        return;

    Camera* cam = Util::getCamera();
    if (!cam) return;

    const Vector3 origin  = cam->Position;
    const Vector3 forward = GetLookForward();

    s_cachedBlk  = GetLookedAtBlock(range, origin, forward);
    s_cachedEnt  = GetLookedAtEntity(origin, forward);
    s_lastUpdate = now;
}

// ── public ────────────────────────────────────────────────────────────────────

void DebugOverlay::RenderLookAtInfo(float x, float y, float fontSize, Color fontColor) {
    LookAtInfo* feature = static_cast<LookAtInfo*>(FeatureHandler::GetFeatureFromName("LookAtInfo"));
    if (!feature || !feature->IsActive()) {
        s_lastUpdate = 0.0;   // reset cache so next enable gets fresh data immediately
        return;
    }

    if (!Util::getGameInstance())
        return;

    UpdateCache(feature->GetRange());

    const float lineHeight = Fonts::Figtree->getHeight() * fontSize + 1.0f;

    if (s_cachedBlk.found)
        Fonts::Figtree->RenderText(
            std::format("Block: x={} y={} z={} id={} ({})",
                s_cachedBlk.x, s_cachedBlk.y, s_cachedBlk.z,
                s_cachedBlk.blockId, s_cachedBlk.name),
            x, y, fontSize, fontColor);
    else
        Fonts::Figtree->RenderText("Block: none", x, y, fontSize, fontColor);

    if (s_cachedEnt.found)
        Fonts::Figtree->RenderText(
            std::format("Entity: x={:.1f} y={:.1f} z={:.1f} id={} ({})",
                s_cachedEnt.position.x, s_cachedEnt.position.y, s_cachedEnt.position.z,
                s_cachedEnt.networkId, s_cachedEnt.name),
            x, y + lineHeight, fontSize, fontColor);
    else
        Fonts::Figtree->RenderText("Entity: none", x, y + lineHeight, fontSize, fontColor);
}
