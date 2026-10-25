#include "scotland2/shared/modloader.h"
#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "paper2_scotland2/shared/logger.hpp"

#include "GlobalNamespace/AudioTimeSyncController.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/PrimitiveType.hpp"
#include "UnityEngine/Renderer.hpp"
#include "UnityEngine/Collider.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/Object.hpp"
#include "UnityEngine/Video/VideoPlayer.hpp"
#include "UnityEngine/Video/VideoSource.hpp"
#include "UnityEngine/Video/VideoRenderMode.hpp"
#include "UnityEngine/Video/VideoAudioOutputMode.hpp"

#include <filesystem>
#include <cmath>
#include <string>

static modloader::ModInfo modInfo{MOD_ID, VERSION, 0};
constexpr auto Logger = Paper::ConstLoggerContext("CinemaLite");

static const std::string VIDEO_PATH =
    "/sdcard/ModData/com.beatgames.beatsaber/Mods/CinemaLite/video.mp4";

static UnityW<UnityEngine::Video::VideoPlayer> player;
static float lastSongTime = -999.f;

MAKE_HOOK_MATCH(ATSC_Start, &GlobalNamespace::AudioTimeSyncController::Start, void,
                GlobalNamespace::AudioTimeSyncController* self) {
    ATSC_Start(self);
    player = nullptr;

    if (!std::filesystem::exists(VIDEO_PATH)) {
        Logger.info("Video not found: {}", VIDEO_PATH);
        return;
    }

    auto go = UnityEngine::GameObject::CreatePrimitive(UnityEngine::PrimitiveType::Quad);
    auto col = go->GetComponent<UnityEngine::Collider*>();
    if (col) UnityEngine::Object::Destroy(col);

    auto t = go->get_transform();
    t->set_position(UnityEngine::Vector3(0.f, 12.f, 80.f));
    t->set_localScale(UnityEngine::Vector3(128.f, 72.f, 1.f));

    auto renderer = go->GetComponent<UnityEngine::Renderer*>();

    UnityW<UnityEngine::Shader> shader;
    for (auto name : {"Sprites/Default", "Unlit/Texture", "UI/Default"}) {
        shader = UnityEngine::Shader::Find(name);
        if (shader) break;
    }
    if (!shader) {
        Logger.error("No usable shader found");
        return;
    }
    renderer->set_material(UnityEngine::Material::New_ctor(shader.ptr()));

    auto vp = go->AddComponent<UnityEngine::Video::VideoPlayer*>();
    vp->set_playOnAwake(false);
    vp->set_source(UnityEngine::Video::VideoSource::Url);
    vp->set_url(StringW("file://" + VIDEO_PATH));
    vp->set_renderMode(UnityEngine::Video::VideoRenderMode::MaterialOverride);
    vp->set_targetMaterialRenderer(renderer);
    vp->set_targetMaterialProperty("_MainTex");
    vp->set_audioOutputMode(UnityEngine::Video::VideoAudioOutputMode::None);
    vp->set_isLooping(false);
    vp->Prepare();
    player = vp;
    lastSongTime = -999.f;
    Logger.info("Cinema screen created");
}

MAKE_HOOK_MATCH(ATSC_Update, &GlobalNamespace::AudioTimeSyncController::Update, void,
                GlobalNamespace::AudioTimeSyncController* self) {
    ATSC_Update(self);
    if (!player || !player->get_isPrepared()) return;

    float t = self->get_songTime();
    bool moving = std::fabs(t - lastSongTime) > 1e-4f;
    lastSongTime = t;

    if (t < 0.f) {
        if (player->get_isPlaying()) player->Pause();
        return;
    }
    if (std::fabs(player->get_time() - (double)t) > 0.25)
        player->set_time((double)t);

    if (moving && !player->get_isPlaying()) player->Play();
    else if (!moving && player->get_isPlaying()) player->Pause();
}

extern "C" __attribute__((visibility("default"))) void setup(CModInfo* info) {
    info->id = MOD_ID;
    info->version = VERSION;
    modInfo.assign(*info);
}

extern "C" __attribute__((visibility("default"))) void late_load() {
    il2cpp_functions::Init();
    INSTALL_HOOK(Logger, ATSC_Start);
    INSTALL_HOOK(Logger, ATSC_Update);
}
