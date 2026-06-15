#pragma once
#include "include/includes.h"
#include "game.h"
#include "game/Ruleset.h"
#include "imgui/inc/8bp.h"
#include "mod/keylogin.h"
#include "oxorany/oxorany.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <sys/system_properties.h>
#include <ctime>
#include <Vector/Vectors.h>
#include <imgui/imgui.h>
#include "icons/icons.h"

using namespace ImGui;
using namespace std;

// ============================================================
// دالة الترجمة (إنجليزي / عربي معكوس)
// ============================================================
static const char* L(const char* en, const char* ar) {
    return (persistent_int["iLang"] == 1) ? ar : en;
}

// ============================================================
// ألوان نيون جديدة
// ============================================================
#define NEON_BLUE       IM_COL32(0, 200, 255, 255)
#define NEON_PURPLE     IM_COL32(155, 48, 255, 255)
#define NEON_PINK       IM_COL32(255, 20, 147, 255)
#define NEON_GOLD       IM_COL32(255, 215, 0, 255)
#define NEON_GREEN      IM_COL32(0, 255, 150, 255)
#define BG_DARK         IM_COL32(8, 6, 16, 255)
#define BG_PANEL        IM_COL32(15, 12, 28, 255)
#define TEXT_WHITE      IM_COL32(255, 255, 255, 255)
#define TEXT_GOLD       IM_COL32(255, 215, 0, 255)

struct MenuState {
    bool isOpen = false;
    int currentTab = 0;
    float sidebarWidth = 750.0f;
    float animProgress = 0.0f;
    float menuAlpha = 0.0f;
    float menuScale = 0.9f;
    ImVec4 accentColor = ImVec4(0.35f, 0.65f, 0.95f, 1.0f);
};
static MenuState g_menu;

static const int64_t EXPIRY_TS = O(1785624451LL);

static bool DEBUG_BYPASS_LOGIN = false;

static float EaseOutBack(float x) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    return 1.0f + c3 * powf(x - 1.0f, 3.0f) + c1 * powf(x - 1.0f, 2.0f);
}

static float EaseOutQuart(float x) {
    return 1.0f - powf(1.0f - x, 4.0f);
}

static void DrawGradientRect(ImDrawList* dl, ImVec2 p1, ImVec2 p2, ImU32 col1, ImU32 col2, bool horizontal = true) {
    if (horizontal) {
        dl->AddRectFilledMultiColor(p1, p2, col1, col2, col2, col1);
    } else {
        dl->AddRectFilledMultiColor(p1, p2, col1, col1, col2, col2);
    }
}

static bool SidebarButton(const char* label, GLuint iconTex, bool selected, float width) {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    float iconSize   = 60.0f;
    float vPad       = 10.0f;
    float btnH       = vPad + iconSize + 4.0f + g.FontSize + vPad;

    ImVec2 pos  = window->DC.CursorPos;
    ImVec2 size = ImVec2(width, btnH);

    const ImRect bb(pos, pos + size);
    ItemSize(size, style.FramePadding.y);
    if (!ItemAdd(bb, id)) return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);

    ImDrawList* dl = window->DrawList;

    float iconBgPad  = 6.0f;
    float iconBgSize = iconSize + iconBgPad * 2.0f;
    ImVec2 iconCenter = ImVec2(
        bb.Min.x + width * 0.5f,
        bb.Min.y + vPad + iconSize * 0.5f
    );

    // تأثير نيون عند التحديد
    if (selected) {
        dl->AddRectFilled(
            ImVec2(iconCenter.x - iconBgSize * 0.5f, iconCenter.y - iconBgSize * 0.5f),
            ImVec2(iconCenter.x + iconBgSize * 0.5f, iconCenter.y + iconBgSize * 0.5f),
            NEON_BLUE, 12.0f
        );
        dl->AddCircle(iconCenter, iconBgSize * 0.6f, NEON_GOLD, 32, 2.5f);
    } else if (hovered) {
        dl->AddRectFilled(
            ImVec2(iconCenter.x - iconBgSize * 0.5f, iconCenter.y - iconBgSize * 0.5f),
            ImVec2(iconCenter.x + iconBgSize * 0.5f, iconCenter.y + iconBgSize * 0.5f),
            IM_COL32(155, 48, 255, 60), 12.0f
        );
    }

    if (iconTex) {
        ImVec2 iconMin = ImVec2(iconCenter.x - iconSize * 0.5f, iconCenter.y - iconSize * 0.5f);
        ImVec2 iconMax = ImVec2(iconCenter.x + iconSize * 0.5f, iconCenter.y + iconSize * 0.5f);
        dl->AddImage((void*)(intptr_t)iconTex, iconMin, iconMax, ImVec2(0,0), ImVec2(1,1));
    }

    ImVec2 labelSize = CalcTextSize(label);
    ImVec2 textPos   = ImVec2(
        bb.Min.x + (width - labelSize.x) * 0.5f,
        bb.Min.y + vPad + iconSize + 4.0f
    );
    ImU32 textCol = selected ? TEXT_WHITE : (hovered ? NEON_GOLD : IM_COL32(140, 140, 150, 255));
    dl->AddText(textPos, textCol, label);

    return pressed;
}

static bool ToggleSwitch(const char* label, bool* v) {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    float scale = 1.5f;
    float height = 32.0f * scale;
    float width = 56.0f * scale;
    float radius = height * 0.5f;

    ImVec2 textSize = CalcTextSize(label);
    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = ImVec2(GetContentRegionAvail().x, ImMax(height, textSize.y) + style.FramePadding.y * 2 + 10.0f);

    const ImRect bb(pos, pos + size);
    ItemSize(size, style.FramePadding.y);
    if (!ItemAdd(bb, id)) return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);
    if (pressed) *v = !*v;

    static std::map<ImGuiID, float> switchAnim;
    float& animT = switchAnim[id];
    float targetT = *v ? 1.0f : 0.0f;
    animT += (targetT - animT) * g.IO.DeltaTime * 14.0f;

    ImDrawList* dl = window->DrawList;
    
    if (hovered) {
        dl->AddRectFilled(bb.Min, bb.Max, IM_COL32(155, 48, 255, 60), 10.0f);
    }
    
    ImVec2 togglePos = ImVec2(bb.Max.x - width - 15.0f, bb.Min.y + (size.y - height) * 0.5f);
    ImVec2 toggleEnd = ImVec2(togglePos.x + width, togglePos.y + height);
    
    ImVec4 offColor = ImVec4(0.18f, 0.15f, 0.25f, 1.0f);
    ImVec4 onColor = ImVec4(0.0f, 0.8f, 1.0f, 1.0f);
    ImVec4 bgColorV = ImLerp(offColor, onColor, animT);
    dl->AddRectFilled(togglePos, toggleEnd, ImColor(bgColorV), radius);
    
    if (*v) {
        dl->AddRect(togglePos, toggleEnd, NEON_BLUE, radius, 0, 2.0f);
    }
    
    float knobX = togglePos.x + radius + (width - height) * animT;
    float knobY = togglePos.y + radius;
    float knobR = radius - 4.0f;
    
    dl->AddCircleFilled(ImVec2(knobX, knobY), knobR + 2.0f, IM_COL32(0, 0, 0, 60));
    dl->AddCircleFilled(ImVec2(knobX, knobY), knobR, TEXT_WHITE);
    
    if (*v) {
        dl->AddCircle(ImVec2(knobX, knobY), knobR + 1.0f, NEON_BLUE, 16, 1.5f);
    }

    dl->AddText(ImVec2(bb.Min.x + 15.0f, bb.Min.y + (size.y - textSize.y) * 0.5f), TEXT_WHITE, label);

    return pressed;
}

// File-scope so DrawToggleButton cancel can also reset countdown
static bool g_aqCounting = false;
static std::chrono::steady_clock::time_point g_aqLastCall;
static std::chrono::steady_clock::time_point g_aqCountdownStart;

static bool IsExpired() {
    return (int64_t)time(nullptr) >= EXPIRY_TS;
}

INLINE void DrawExpired(ImGuiIO& io) {
    float winW = g_menu.sidebarWidth;

    SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    SetNextWindowSize(ImVec2(winW, 0), ImGuiCond_Always);
    PushStyleColor(ImGuiCol_WindowBg, NEON_BLUE);
    PushStyleVar(ImGuiStyleVar_WindowRounding, 20.0f);
    PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(30.0f, 30.0f));
    PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    if (Begin(O("##ExpiredWin"), nullptr,
              ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
              ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
              ImGuiWindowFlags_AlwaysAutoResize)) {

        SetWindowFontScale(1.6f);
        ImVec2 titleSz = CalcTextSize(L("MOD EXPIRED", "ﻲﻫﺎﻨﺘﻣ ﻞﻳﺪﻌﺘﻟﺍ"));
        SetCursorPosX((winW - 60.0f - titleSz.x) * 0.5f);
        TextColored(ImVec4(1.0f, 0.1f, 0.1f, 1.0f), "%s", L("MOD EXPIRED", "ﻲﻫﺎﻨﺘﻣ ﻞﻳﺪﻌﺘﻟﺍ"));
        SetWindowFontScale(1.0f);

        Dummy(ImVec2(0, 16));

        PushTextWrapPos(GetCursorPosX() + winW - 60.0f);
        TextColored(ImVec4(0.85f, 0.85f, 0.90f, 1.0f), "%s",
            L("Beta Version Expired. Update on our Telegram @DRAGON_GYMAR", "ﺔﺨﺴﻨﻟﺍ ﺖﻬﺘﻧﺍ .ﺚﺣﺍﺮﺑ ﻞﻳﺪﻌﺘﻟﺍ ﻊﻓﺭﺍ @DRAGON_GYMAR"));
        PopTextWrapPos();

        Dummy(ImVec2(0, 10));
    }
    End();
    PopStyleVar(3);
    PopStyleColor();
}

INLINE void DrawAutoQueue() {
    if ((!g_Token.empty() && !g_Auth.empty() && g_Token == g_Auth) || DEBUG_BYPASS_LOGIN) {
        
        // إذا لم يبدأ العد بعد، نقوم ببدئه مرة واحدة فقط
        if (!g_aqCounting) {
            g_aqCounting = true;
            g_aqCountdownStart = std::chrono::steady_clock::now();
        }

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - g_aqCountdownStart).count();
        int remaining_ms = 3000 - (int)elapsed;

        // عند انتهاء العد التنازلي
        if (remaining_ms <= 0) {
            // ========== تنفيذ الدخول ==========
            int mode = persistent_int["iAutoQueue_Mode"];
            
            // وضع آخر مختار
            if (mode == 0 && lastMatchInfo.set) {
                LOGI("Joining last match: %s", lastMatchInfo.Tier.c_str());
                _StartMatch(sharedMenuManager.instance, 0, lastMatchInfo.Tier, 
                            0, 0, 0, 0, 0, 0, lastMatchInfo.arg10, lastMatchInfo.arg11);
            }
            // وضع ذكي
            else if (mode == 1) {
                auto coins = sharedUserInfo.coins();
                auto maxBet = coins * persistent_int["iAutoQueue_BetPercent"] / 100;
                
                std::map<string, int64_t> modeBets = {
                    {"M1", 50}, {"M2", 100}, {"M3", 500}, {"M4", 2500},
                    {"M5", 10000}, {"M6", 50000}, {"M7", 100000}, {"M8", 250000},
                    {"M9", 500000}, {"M10", 1000000}, {"M11", 2500000}, {"M12", 4000000},
                    {"M13", 5000000}, {"M14", 10000000}, {"M15", 15000000}, {"M16", 25000000},
                    {"M17", 100000000}
                };
                
                string selectedMode = "M1";
                for (const auto& [mn, bet] : modeBets) {
                    if (maxBet >= bet) selectedMode = mn;
                }
                
                LOGI("Smart mode: %s", selectedMode.c_str());
                if (lastMatchInfo.set) {
                    _StartMatch(sharedMenuManager.instance, 0, selectedMode, 
                                0, 0, 0, 0, 0, 0, lastMatchInfo.arg10, lastMatchInfo.arg11);
                } else {
                    _StartMatch(sharedMenuManager.instance, 0, selectedMode, 
                                0, 0, 0, 0, 0, 0, 0x7100000001, 0xffffffff);
                }
            }
            
            // إيقاف العد لمنع التكرار
            g_aqCounting = false;
            // اختيارياً: يمكنك إيقاف الخاصية بالكامل بعد الانضمام إذا أردت
            // persistent_bool[O("bAutoQueue")] = false; 
            return;
        }

        // عرض العد التنازلي على الشاشة
        SetNextWindowPos(ImVec2(Width * 0.5f, Height * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 1.f));
        PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(32.0f, 20.0f));
        PushStyleVar(ImGuiStyleVar_WindowRounding, 24.0f);

        if (Begin(O("##AutoQueueCD"), nullptr,
                  ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                  ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
                  ImGuiWindowFlags_AlwaysAutoResize)) {
            ImDrawList* dl = GetWindowDrawList();
            ImVec2 wp = GetWindowPos();
            ImVec2 ws = GetWindowSize();
            dl->AddRectFilled(wp, ImVec2(wp.x + ws.x, wp.y + ws.y), IM_COL32(20, 20, 28, 0), 24.0f);
            dl->AddRect(wp, ImVec2(wp.x + ws.x, wp.y + ws.y), NEON_GOLD, 24.0f, 0, 2.0f);

            SetWindowFontScale(3.5f);
            // إضافة 1 لجعل العد من 3 إلى 1
            std::string count_str = std::to_string((remaining_ms / 1000) + 1);
            TextColored(ImVec4(0.83f, 0.68f, 0.22f, 1.0f), "%s", count_str.c_str());
            SetWindowFontScale(1.0f);
        }
        End();
        PopStyleVar(2);
        PopStyleColor();
    }
}


#include "mod/ButtonClicker.h"

static void DrawToggleButton(bool cancelMode); // forward declaration — defined after DrawFloatingButton

INLINE void DrawESP(ImDrawList* draw) {
    if ((!g_Token.empty() && !g_Auth.empty() && g_Token == g_Auth) || DEBUG_BYPASS_LOGIN) {
        if (!sharedGameManager) return;

        UpdateScreenTable();

        sharedDirector = F(ptr, libmain + O(0x4f06288));
        if (!sharedDirector) return;

        sharedUserInfo = F(ptr, libmain + O(0x4e9feb8));
        if (!sharedUserInfo) return;

        F(bool, sharedUserInfo + 0x340) = true;

        sharedMainManager = F(ptr, libmain + O(0x4dde3e0));
        if (!sharedMainManager) return;

        sharedMenuManager = F(ptr, libmain + O(0x4dfe838));
        if (!sharedMenuManager) return;

        MainStateManager mainStateManager = sharedMainManager.mStateManager;
        if (!mainStateManager) return;
        if (!mainStateManager.isInGame()) {
        if (persistent_bool[O("bAutoQueue")]) {
            if (!sharedMenuManager.isInQueue()) DrawAutoQueue();
            DrawToggleButton(true);  // acts as cancel button for autoqueue
        } return;
        }

        auto visualCue = sharedGameManager.mVisualCue();

        Ball::Classification myclass = sharedGameManager.getPlayerClassification();

        Table table = sharedGameManager.mTable;
        if (!table) return;

        auto tableProperties = table.mTableProperties();
        if (!tableProperties) return;

        auto& pockets = tableProperties.mPockets();

        GameStateManager gameStateManager = sharedGameManager.mStateManager;
        if (!gameStateManager) return;

        if (persistent_bool[O("bAutoPlay")]) {
            DrawToggleButton(false);
        AutoPlay::Update();
        }

        auto stateId = gameStateManager.getCurrentStateId();
        if (stateId == 4) gPrediction->determineShotResult(false);
        if (stateId == 6 || stateId == 7 || stateId == 8) return;

        if (persistent_bool[O("bESP_DrawPocketsShotState")]) {
            for (int i = 0; i < 6; i++) {
                if (Prediction::pocketStatus[i]) {
                    auto screenPos = WorldToScreen(pockets[i]);
                    draw->AddCircle(ImVec2(screenPos.x, screenPos.y), 30, NEON_GREEN, 0, 5.f);
                }
            }
        }

        if (persistent_bool[O("bESP_DrawPredictionLine")]) {
            for (int i = 0; i < gPrediction->guiData.ballsCount; i++) {
                auto& ball = gPrediction->guiData.balls[i];

                if (ball.initialPosition != ball.predictedPosition) {
                    ImVec2 lastPos{};
                    float lineThick = (float)persistent_int[O("iLineThickness")];
                    if (lineThick < 1.f) lineThick = 1.f;
                    for (int j = 1; j < ball.positions.size(); j++) {
                        auto point = WorldToScreen(ball.positions[j]);
                        if (lastPos.x || lastPos.y) draw->AddLine(lastPos, point, colors[i], lineThick);
                        lastPos = point;
                    }
                }
            }
        }

        if (persistent_bool[O("bESP_DrawPredictionLine")]) {
            for (int i = 0; i < gPrediction->guiData.ballsCount; i++) {
                auto& ball = gPrediction->guiData.balls[i];

                if (ball.initialPosition != ball.predictedPosition) {
                    float circleR = (float)persistent_int[O("iLineThickness")] + 1.f;
                    if (circleR < 2.f) circleR = 2.f;
                    draw->AddCircleFilled(WorldToScreen(ball.initialPosition), circleR, colors[i]);
                    draw->AddCircleFilled(WorldToScreen(ball.predictedPosition), 16, colors[i]);
                }
            }
        }
    }
}

static void DrawSidebar(float sidebarW) {
    static GLuint draw_icon_tex = LoadTextureFromMemory(draw_icon_png, draw_icon_png_len);
    static GLuint play_icon_tex = LoadTextureFromMemory(play_icon_png, play_icon_png_len);
    static GLuint q_icon_tex    = LoadTextureFromMemory(q_icon_png,    q_icon_png_len);
    static GLuint user_icon_tex = LoadTextureFromMemory(user_icon_png, user_icon_png_len);

    ImGuiContext& g  = *GImGui;
    ImDrawList*   dl = GetWindowDrawList();
    ImVec2        wp = GetWindowPos();

    float closeSize = 35.0f;
    float closeBtnW = 70.0f;
    float tabsW     = sidebarW - closeBtnW;
    float btnW      = tabsW / 4.0f;
    float marginB   = 12.0f;

    dl->ChannelsSplit(2);
    dl->ChannelsSetCurrent(1);

    BeginGroup();
    SetCursorPos(ImVec2(0.0f, 0.0f));
    if (SidebarButton(L("amendment", "تادادعإ"), draw_icon_tex, g_menu.currentTab == 0, btnW)) g_menu.currentTab = 0;
    SameLine(0, 0);
    if (SidebarButton(L("Auto PLAY", "ﻲﺋﺎﻘﻠﺘﻟﺍ ﺐﻌﻠﻟﺍ"), play_icon_tex, g_menu.currentTab == 1, btnW)) g_menu.currentTab = 1;
    SameLine(0, 0);
    if (SidebarButton(L("Auto Queue", "ﻲﺋﺎﻘﻠﺘﻟﺍ ﺭﺎﻈﺘﻧﻻﺍ"), q_icon_tex, g_menu.currentTab == 2, btnW)) g_menu.currentTab = 2;
    SameLine(0, 0);
    if (SidebarButton(L("Information", "تﺎﻣﻮﻠﻌﻤﻟﺍ"), user_icon_tex, g_menu.currentTab == 3, btnW)) g_menu.currentTab = 3;
    EndGroup();

    float sidebarH = GetItemRectMax().y - wp.y;

    dl->ChannelsSetCurrent(0);
    dl->AddRectFilled(wp, ImVec2(wp.x + sidebarW, wp.y + sidebarH), BG_DARK, 30.0f);
    dl->AddRect(wp, ImVec2(wp.x + sidebarW, wp.y + sidebarH), NEON_PURPLE, 30.0f, 0, 1.5f);
    dl->ChannelsMerge();

    float sepX       = wp.x + sidebarW - closeBtnW;
    float sepCenterY = wp.y + sidebarH * 0.5f;
    float sepHalfH   = sidebarH * 0.28f;
    dl->AddLine(
        ImVec2(sepX, sepCenterY - sepHalfH),
        ImVec2(sepX, sepCenterY + sepHalfH),
        NEON_GOLD, 2.0f
    );

    float closePosX = (sidebarW - closeBtnW) + (closeBtnW - closeSize) * 0.5f;
    float closePosY = (sidebarH - closeSize) * 0.5f;
    SetCursorPos(ImVec2(closePosX, closePosY));
    {
        ImGuiWindow* win = GetCurrentWindow();
        ImGuiID closeId  = win->GetID(O("##CloseMenu"));
        ImVec2 closePos  = win->DC.CursorPos;
        ImRect closeBb(closePos, closePos + ImVec2(closeSize, closeSize));
        ItemSize(ImVec2(closeSize, closeSize), g.Style.FramePadding.y);
        ItemAdd(closeBb, closeId);
        bool closeHovered = false, closeHeld = false;
        bool closePressed = ButtonBehavior(closeBb, closeId, &closeHovered, &closeHeld);
        if (closePressed) g_menu.isOpen = false;

        float xCX = closeBb.Min.x + closeSize * 0.5f;
        float xCY = closeBb.Min.y + closeSize * 0.5f;
        float xH  = closeSize * 0.32f;
        ImU32 xCol = closeHovered ? NEON_GOLD : TEXT_WHITE;
        dl->AddLine(ImVec2(xCX - xH, xCY - xH), ImVec2(xCX + xH, xCY + xH), xCol, 2.2f);
        dl->AddLine(ImVec2(xCX + xH, xCY - xH), ImVec2(xCX - xH, xCY + xH), xCol, 2.2f);
    }

    SetCursorPos(ImVec2(0.0f, sidebarH));
    Dummy(ImVec2(sidebarW, marginB));
}

// Reads an IL2CPP/Unity NSString (UTF-16 internal buffer at offset 0x14, length at 0x10)
static std::string ReadNSString(ptr str) {
    if (!str) return "null";
    int32_t len = F(int32_t, str + 0x10);
    if (len <= 0 || len > 512) return "?";
    std::string result;
    result.reserve(len);
    for (int32_t i = 0; i < len; i++) {
        uint16_t ch = F(uint16_t, str + 0x14 + i * 2);
        result += (ch > 0 && ch < 128) ? (char)ch : '?';
    }
    return result;
}

// Shared vertical position for DrawToggleButton and DrawFloatingButton (they move together)
static float g_sideBtnsY      = 0.0f;
// Kept for linker compatibility — no longer used for animation
static float g_toggleRotAngle = 0.0f;
// Set true by AutoPlay when in SLOW scan state — shows CALCULATING overlay
static bool  g_autoPlayCalculating = false;

// ── svConfig ──────────────────────────────────────────────────────────────────
static void svConfig_Save() {
    std::string path = O("/data/user/0/") + PACKAGE_NAME + O("/files/svConfig.txt");
    FILE* f = fopen(path.c_str(), O("w"));
    if (!f) return;
    fprintf(f, O("iLineThickness=%d\n"),  persistent_int[O("iLineThickness")]);
    fprintf(f, O("iMenuSizeOffset=%d\n"), persistent_int[O("iMenuSizeOffset")]);
    fclose(f);
}
static void svConfig_Load() {
    std::string path = O("/data/user/0/") + PACKAGE_NAME + O("/files/svConfig.txt");
    FILE* f = fopen(path.c_str(), O("r"));
    if (!f) return;
    char line[64];
    while (fgets(line, sizeof(line), f)) {
        int v = 0;
        if (sscanf(line, O("iLineThickness=%d"),  &v) == 1) { persistent_int[O("iLineThickness")]  = v; continue; }
        if (sscanf(line, O("iMenuSizeOffset=%d"), &v) == 1) { persistent_int[O("iMenuSizeOffset")] = v; }
    }
    fclose(f);
}

// ── CALCULATING overlay (shown during AutoPlay SLOW scan) ─────────────────────
static void DrawCalculating(ImGuiIO& io) {
    SetNextWindowPos(ImVec2(Width * 0.5f, Height * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    
    PushStyleColor(ImGuiCol_WindowBg, NEON_BLUE);
    PushStyleColor(ImGuiCol_Border, NEON_PURPLE);
    PushStyleVar(ImGuiStyleVar_WindowRounding, 18.0f);
    PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);

    if (Begin(O("##CalcOverlay"), nullptr,
              ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
              ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | 
              ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoInputs)) {
        
        SetWindowFontScale(1.4f);
        TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), L("CALCULATING...", "...ﺏﺎﺴﺤﻟﺍ ﻱﺭﺎﺟ"));
        SetWindowFontScale(1.0f);
    }
    End();
    PopStyleVar(2);
    PopStyleColor(2);
}

static void DrawContentArea(float winW, float winH) {
    bool need_save = false;
    
    ImDrawList* dl  = GetWindowDrawList();
    ImVec2      wp  = GetWindowPos();

    float startY   = GetCursorPosY();
    float contentW = winW;

    dl->AddRectFilled(
        ImVec2(wp.x, wp.y + startY),
        ImVec2(wp.x + contentW, wp.y + winH),
        BG_PANEL, 20.0f
    );
    dl->AddRect(
        ImVec2(wp.x, wp.y + startY),
        ImVec2(wp.x + contentW, wp.y + winH),
        NEON_PURPLE, 20.0f, 0, 1.5f
    );
    
    const char* tabTitles[] = { 
        L("Draw Settings", "ﻢﺳﺮﻟﺍ ﺕﺍﺩﺍﺪﻋﺇ"),
        L("Auto Play", "ﻲﺋﺎﻘﻠﺘﻟﺍ ﺐﻌﻠﻟﺍ"),
        L("Auto Queue", "ﻲﺋﺎﻘﻠﺘﻟﺍ ﺭﺎﻈﺘﻧﻻﺍ"),
        L("User", "ﻡﺪﺨﺘﺴﻤﻟﺍ") 
    };

    const char* currentTitle = tabTitles[g_menu.currentTab];
    float titlePadT = 18.0f;
    float titlePadB = 12.0f;

    SetWindowFontScale(1.15f);
    ImVec2 ts = CalcTextSize(currentTitle);
    
    float centeredX = (contentW - ts.x) * 0.5f;
    SetCursorPos(ImVec2(centeredX, startY + titlePadT));
    
    TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "%s", currentTitle);
    SetWindowFontScale(1.0f);

    float lineY = startY + titlePadT + ts.y + titlePadB;
    dl->AddLine(
        ImVec2(wp.x + 20.0f, wp.y + lineY),
        ImVec2(wp.x + contentW - 20.0f, wp.y + lineY),
        NEON_GOLD, 1.5f
    );

    float headerH = (lineY - startY) + 10.0f;
    SetCursorPos(ImVec2(10.0f, startY + headerH));
    
    PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
    BeginChild(O("##ContentArea"), ImVec2(contentW - 20.0f, winH - startY - headerH - 10.0f), false);
    
    switch (g_menu.currentTab) {
        case 0: {
            Dummy(ImVec2(0, 10));
            
            // زر اللغة
            TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "%s", L("Language", "ﺔﻐﻠﻟﺍ"));
            Dummy(ImVec2(0, 8));
            int curLang = persistent_int["iLang"];
            float bw = (GetContentRegionAvail().x - 10) * 0.5f;
            PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
            PushStyleColor(ImGuiCol_Button,        curLang==0 ? (ImVec4)ImColor(NEON_BLUE) : ImVec4(0.12f, 0.12f, 0.15f, 1.0f));
            PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.20f, 0.30f, 1.0f));
            PushStyleColor(ImGuiCol_Text,          curLang==0 ? ImVec4(1,1,1,1) : ImVec4(0.7f,0.75f,0.85f,1));
            if (Button("English", ImVec2(bw, 44))) { persistent_int["iLang"] = 0; need_save = true; }
            PopStyleColor(3);
            SameLine();
            PushStyleColor(ImGuiCol_Button,        curLang==1 ? (ImVec4)ImColor(NEON_BLUE) : ImVec4(0.12f, 0.12f, 0.15f, 1.0f));
            PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.20f, 0.30f, 1.0f));
            PushStyleColor(ImGuiCol_Text,          curLang==1 ? ImVec4(1,1,1,1) : ImVec4(0.7f,0.75f,0.85f,1));
            if (Button(L("العربية", "ﺔﻴﺑﺮﻌﻟﺍ"), ImVec2(bw, 44))) { persistent_int["iLang"] = 1; need_save = true; }
            PopStyleColor(3);
            PopStyleVar();
            Dummy(ImVec2(0, 20));
            
            need_save |= ToggleSwitch(L("Draw Lines", "ﻂﺧ ﻢﺳﺭ"), &persistent_bool[O("bESP_DrawPredictionLine")]);
            need_save |= ToggleSwitch(L("Draw Pockets", "ﺏﻮﻴﺠﻟﺍ ﻢﺳﺭ"), &persistent_bool[O("bESP_DrawPocketsShotState")]);

            Dummy(ImVec2(0, 16));
            TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), L("Line Thickness", "ﻂﺨﻟﺍ ﻚﻤﺳ"));
            Dummy(ImVec2(0, 8));
            {
                if (persistent_int[O("iLineThickness")] < 1) persistent_int[O("iLineThickness")] = 4;
                PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
                PushStyleVar(ImGuiStyleVar_GrabRounding, 10.0f);
                PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.12f, 0.12f, 0.15f, 1.0f));
                PushStyleColor(ImGuiCol_SliderGrab, NEON_BLUE);
                PushStyleColor(ImGuiCol_SliderGrabActive, NEON_PURPLE);
                SetNextItemWidth(GetContentRegionAvail().x);
                need_save |= SliderInt(O("##lineThick"), &persistent_int[O("iLineThickness")], 1, 10, "%d");
                PopStyleColor(3);
                PopStyleVar(2);
            }

            Dummy(ImVec2(0, 16));
            TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), L("Fix Menu Size", "ﺔﻤﺋﺎﻘﻟﺍ ﻢﺠﺣ ﺖﻴﺒﺜﺗ"));
            Dummy(ImVec2(0, 8));
            {
                PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
                PushStyleVar(ImGuiStyleVar_GrabRounding, 10.0f);
                PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.12f, 0.12f, 0.15f, 1.0f));
                PushStyleColor(ImGuiCol_SliderGrab, NEON_BLUE);
                PushStyleColor(ImGuiCol_SliderGrabActive, NEON_PURPLE);
                SetNextItemWidth(GetContentRegionAvail().x);
                int& menuSz = persistent_int[O("iMenuSizeOffset")];
                bool changed = SliderInt(O("##menuSize"), &menuSz, -10, 10,
                    menuSz == 0 ? L("Normal", "ﻱﺩﺎﻋ") : "%d");
                need_save |= changed;
                PopStyleColor(3);
                PopStyleVar(2);
            }

            Dummy(ImVec2(0, 20));
            {
                PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
                PushStyleColor(ImGuiCol_Button,        NEON_BLUE);
                PushStyleColor(ImGuiCol_ButtonHovered, NEON_PURPLE);
                PushStyleColor(ImGuiCol_ButtonActive,  NEON_PINK);
                if (Button(L("Save Config", "ﺕﺍﺩﺍﺪﻋﻹﺍ ﻆﻔﺣ"), ImVec2(GetContentRegionAvail().x, 55.0f))) {
                    svConfig_Save();
                }
                PopStyleColor(3);
                PopStyleVar();
            }
            break;
        }
        
        case 1: {
            Dummy(ImVec2(0, 10));
            need_save |= ToggleSwitch(L("Enable AutoPlay", "ﻲﺋﺎﻘﻠﺘﻟﺍ ﺐﻌﻠﻟﺍ ﻞﻴﻌﻔﺗ"), &persistent_bool[O("bAutoPlay")]);
            Dummy(ImVec2(0, 20));
            TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1.0f), L("Auto Play Will Automatically", "ﻲﺋﺎﻘﻠﺗ ﻡﻮﻘﻴﺳ ﻲﺋﺎﻘﻠﺘﻟﺍ ﺐﻌﻠﻟﺍ"));
            TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1.0f), L("Aim And Shoot For You", "ﻚﻨﻋ ًﻻﺪﺑ ﺐﻌﻠﻟﺍﻭ ﺐﻳﻮﺼﺘﻟﺎﺑ"));
            break;
        }
        
        case 2: {
            Dummy(ImVec2(0, 10));
            need_save |= ToggleSwitch(L("Enable AutoQueue", "ﻲﺋﺎﻘﻠﺘﻟﺍ ﺭﺎﻈﺘﻧﻻﺍ ﻞﻴﻌﻔﺗ"), &persistent_bool[O("bAutoQueue")]);
            Dummy(ImVec2(0, 20));
            
            TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), L("Mode", "ﻊﺿﻮﻟﺍ"));
            Dummy(ImVec2(0, 8));
            PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
            PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(15, 12));
            PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.12f, 0.12f, 0.15f, 1.0f));
            PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.16f, 0.16f, 0.20f, 1.0f));
            SetNextItemWidth(GetContentRegionAvail().x);
            need_save |= Combo("##mode", &persistent_int["iAutoQueue_Mode"], L("Last Selected\0Smart\0", "ﺭﺎﻴﺘﺧﻻﺍ ﺮﻴﺧﻷﺍ\0ﻲﻛﺫ\0"));
            PopStyleColor(2);
            PopStyleVar(2);
            
            if (persistent_int["iAutoQueue_Mode"] == 1) {
                Dummy(ImVec2(0, 15));
                TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), L("Bet Percent", "ﻥﺎﻫﺮﻟﺍ ﺔﺒﺴﻧ"));
                Dummy(ImVec2(0, 8));
                PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
                PushStyleVar(ImGuiStyleVar_GrabRounding, 10.0f);
                PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.12f, 0.12f, 0.15f, 1.0f));
                PushStyleColor(ImGuiCol_SliderGrab, NEON_BLUE);
                PushStyleColor(ImGuiCol_SliderGrabActive, NEON_PURPLE);
                SetNextItemWidth(GetContentRegionAvail().x);
                need_save |= SliderInt("##betpercent", &persistent_int["iAutoQueue_BetPercent"], 1, 100, "%d%%");
                PopStyleColor(3);
                PopStyleVar(2);
            }

            if (persistent_int["iAutoQueue_Mode"] == 0) {
                Dummy(ImVec2(0, 15));
                TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1.0f), L("You Will Be Auto Queued To", "ﻰﻟﺇ ﻲﺋﺎﻘﻠﺗ ﻞﺧﺪﺘﺳ"));
                TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1.0f), L("The Last Game Mode You Played", "ﺖﺒﻌﻟ ﺮﺧﻵﺍ ﻊﺿﻮﻟﺍ"));
            }
            break;
        }

        case 3: {
            // ── helpers ──────────────────────────────────────────────────────
            auto DrawSectionHeader = [&](const char* title) {
                Dummy(ImVec2(0, 14));
                float avail = GetContentRegionAvail().x;
                ImVec2 p    = GetCursorScreenPos();
                float  fs   = GImGui->FontSize;
                ImVec2 ts   = CalcTextSize(title);
                float  lineY = p.y + fs * 0.5f;
                float  gap   = 8.0f;
                float  lineW = (avail - ts.x - gap * 2.0f) * 0.5f;
                ImDrawList* dl2 = GetWindowDrawList();
                dl2->AddLine(ImVec2(p.x,                      lineY), ImVec2(p.x + lineW,                      lineY), NEON_BLUE, 1.0f);
                dl2->AddLine(ImVec2(p.x + lineW + gap + ts.x + gap, lineY), ImVec2(p.x + avail, lineY), NEON_BLUE, 1.0f);
                SetCursorPosX(GetCursorPosX() + lineW + gap);
                TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "%s", title);
                Dummy(ImVec2(0, 6));
            };

            auto DrawInfoRow = [&](const char* key, const char* val) {
                TextColored(ImVec4(0.55f, 0.55f, 0.65f, 1.0f), "%s", key);
                SameLine();
                TextColored(ImVec4(0.90f, 0.90f, 0.95f, 1.0f), "%s", val);
                Dummy(ImVec2(0, 4));
            };

            // ── Device Info ───────────────────────────────────────────────────
            DrawSectionHeader(L("System Status", "ﻡﺎﻈﻨﻟﺍ ﺔﻟﺎﺣ"));
            {
                static char s_manufacturer[PROP_VALUE_MAX] = {};
                static char s_model[PROP_VALUE_MAX] = {};
                static char s_abi[PROP_VALUE_MAX] = {};
                static char s_android[PROP_VALUE_MAX] = {};
                static bool s_props_loaded = false;
                
                if (!s_props_loaded) {
                    __system_property_get("ro.product.manufacturer", s_manufacturer);
                    __system_property_get("ro.product.model", s_model);
                    __system_property_get("ro.product.cpu.abi", s_abi);
                    __system_property_get("ro.build.version.release", s_android);
                    s_props_loaded = true;}
                    
                int64_t now_ts = (int64_t)time(nullptr);
                int64_t diff = EXPIRY_TS - now_ts;
                char expireBuf[64];
                if (diff > 0) {
                    int64_t totalSecs = diff;
                    int days  = (int)(totalSecs / 86400);
                    int hours = (int)((totalSecs % 86400) / 3600);
                    int mins  = (int)((totalSecs % 3600)  / 60);
                    snprintf(expireBuf, sizeof(expireBuf), "%d %s - %d %s - %d %s", days, L("days", "مﻮﻳ"), hours, L("hours", "ةﺎﻋ"), mins, L("mins", "ةﻘﻳﺪﻟ"));
                } else {
                snprintf(expireBuf, sizeof(expireBuf), "%s", L("Expired", "ﻲﻫﺎﻨﺘﻣ"));}
                    
                DrawInfoRow(L("Manufacturer: ", "ﻊﻨﺼﻤﻟﺍ: "), s_manufacturer);
                DrawInfoRow(L("Model: ", "ﺯﺍﺮﻄﻟﺍ: "), s_model);
                DrawInfoRow(L("ABI: ", "ABI: "), s_abi);
                DrawInfoRow(L("Android: ", "ﺪﻳﻭﺮﻛﻷﺍ: "), s_android);
                DrawInfoRow(L("Key: ", "ﺡﺎﺘﻔﻤﻟﺍ: "), persistent_string["key"].c_str());
                DrawInfoRow(L("Expiry: ", "ﻱﺎﻬﺘﻧﻻﺍ ﺦﻳﺭﺎﺗ: "), expireBuf);}
            break;
        }
    }
    
    if (need_save) save_persistence();
    
    EndChild();
    PopStyleColor();
}

INLINE void DrawMenu(ImGuiIO& io) {
    if ((!g_Token.empty() && !g_Auth.empty() && g_Token == g_Auth) || DEBUG_BYPASS_LOGIN) {
        if (is_segv_handler_active()) {
            jump_buffer_active = 1;
            if (!sigsetjmp(jump_buffer, 1)) DrawESP(GetBackgroundDrawList());
            jump_buffer_active = 0;
        }

        float targetAlpha = g_menu.isOpen ? 1.0f : 0.0f;
        if (g_menu.isOpen) {
            g_menu.menuAlpha += (1.0f - g_menu.menuAlpha) * io.DeltaTime * 12.0f;
        } else {
            g_menu.menuAlpha = 0.0f;
        }

        if (g_menu.menuAlpha > 0.01f) {
            float sizeScale = 1.0f + (float)persistent_int[O("iMenuSizeOffset")] * 0.03f;
            if (sizeScale < 0.3f) sizeScale = 0.3f;
            float winW = g_menu.sidebarWidth * sizeScale;
            float winH = 560.0f * sizeScale;
            
            SetNextWindowSize(ImVec2(winW, winH), ImGuiCond_Always);
            SetNextWindowPos(ImVec2(Width / 2.0f, Height / 2.0f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            
            PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.10f, 0.13f, 0.f));
            PushStyleVar(ImGuiStyleVar_WindowRounding, 16.0f);
            PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            PushStyleVar(ImGuiStyleVar_Alpha, g_menu.menuAlpha);
            
            ImGuiWindowFlags winFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
                                        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
                                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            
            if (Begin(O("##MainMenu"), &g_menu.isOpen, winFlags)) {
                DrawSidebar(winW);
                DrawContentArea(winW, winH);
            }
            End();
            
            PopStyleVar(4);
            PopStyleColor();
        }
    }
}

// ــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــ //

static void DrawToggleButton(bool cancelMode) {
    if (g_menu.isOpen) return;

    ImGuiIO& io = GetIO();
    static GLuint play_on_tex = LoadTextureFromMemory(play_on_png, play_on_png_len);
    static GLuint play_off_tex = LoadTextureFromMemory(play_off_png, play_off_png_len);
    static GLuint queue_cancel_tex = LoadTextureFromMemory(play_on_png, play_on_png_len);

    float button_size = 130.f;
    float windowWidth = button_size + GetStyle().WindowPadding.x * 2.0f;
    float windowHeight = button_size + GetStyle().WindowPadding.y * 2.0f;
    const float rightMargin = 20.0f;

    float fixedX = io.DisplaySize.x - rightMargin - windowWidth;

    SetNextWindowSize(ImVec2(windowWidth, windowHeight), ImGuiCond_Always);
    SetNextWindowPos(ImVec2(fixedX, g_sideBtnsY), ImGuiCond_Always);

    PushStyleColor(ImGuiCol_WindowBg, IM_COL32(0, 0, 0, 0));
    PushStyleColor(ImGuiCol_Border,   IM_COL32(0, 0, 0, 0));
    PushStyleVar(ImGuiStyleVar_WindowRounding, 99.0f);

    if (Begin(O("##ToggleBtn"), nullptr,
              ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
              ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove)) {

        ImVec2 pos = GetCursorScreenPos();
        ImVec2 size(button_size, button_size);
        ImVec2 center(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f);

        if (InvisibleButton(O("##TglBtnHit"), size)) {
             if (cancelMode) {
                persistent_bool[O("bAutoQueue")] = false;
                g_aqCounting = false;
            } else {
                AutoPlay::bAutoPlaying = !AutoPlay::bAutoPlaying;
                if (AutoPlay::bAutoPlaying) AutoPlay::ClearState();
            }
        }

        GLuint tex;
        if (cancelMode) tex = queue_cancel_tex;
        else tex = AutoPlay::bAutoPlaying ? play_on_tex : play_off_tex;

        float r = size.x * 0.5f;
        ImDrawList* dl = GetWindowDrawList();
        dl->AddImage((void*)(intptr_t)tex, ImVec2(center.x - r, center.y - r), ImVec2(center.x + r, center.y + r));
    }
    End();
    PopStyleVar();
    PopStyleColor(2);
}

// ــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــ //

static void DrawFloatingButton(ImGuiIO& io) {
    if (g_menu.isOpen) return;

    static GLuint logo_tex = LoadTextureFromMemory(logo_png, logo_png_len);
    
    float buttonRadius = 65.0f;
    float winSize = (buttonRadius * 2.0f) + 10.0f;
    const float rightMargin = 20.0f;

    if (g_sideBtnsY <= 0.0f) g_sideBtnsY = io.DisplaySize.y - 150.0f;

    float toggleWidth = 130.f + (GetStyle().WindowPadding.x * 2.0f);
    float fixedX = io.DisplaySize.x - rightMargin - toggleWidth + (toggleWidth - winSize) * 0.5f;
    
    float posY = g_sideBtnsY - 140.0f; 

    SetNextWindowPos(ImVec2(fixedX, posY), ImGuiCond_Always);
    SetNextWindowSize(ImVec2(winSize, winSize), ImGuiCond_Always);
    
    PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    if (Begin(O("##FloatBtn"), nullptr,
              ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar |
              ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {

        ImDrawList* dl = GetWindowDrawList();
        ImVec2 center = ImVec2(fixedX + (winSize * 0.5f), posY + (winSize * 0.5f));

        InvisibleButton(O("##FloatBtnHit"), ImVec2(winSize, winSize));

        if (IsItemActive() && IsMouseDragging(ImGuiMouseButton_Left)) {
            g_sideBtnsY += io.MouseDelta.y;
            g_sideBtnsY = ImClamp(g_sideBtnsY, 160.0f, io.DisplaySize.y - 150.0f);
        }

        if (IsItemHovered() && IsMouseReleased(0) && ImGui::GetMouseDragDelta(0).y == 0) {
            g_menu.isOpen = true;
        }

        dl->AddImage((void*)(intptr_t)logo_tex,
                     ImVec2(center.x - buttonRadius, center.y - buttonRadius),
                     ImVec2(center.x + buttonRadius, center.y + buttonRadius));
    }
    End();
    PopStyleVar(2);
    PopStyleColor();
}

// ــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــ //

static bool first_time = true;
INLINE void DrawLogin(ImGuiIO& io) {
    if (logged_in) return DrawMenu(io);

    SetNextWindowPos(ImVec2(0, 0));
    SetNextWindowSize(io.DisplaySize);
    PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.04f, 0.06f, 0.96f));
    Begin(O("##Overlay"), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBringToFrontOnFocus);
    PopStyleColor();

    float cardW = 580;
    float cardH = 420;

    SetNextWindowSize(ImVec2(cardW, cardH), ImGuiCond_Always);
    SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    PushStyleColor(ImGuiCol_WindowBg, BG_DARK);
    PushStyleVar(ImGuiStyleVar_WindowRounding, 35.0f);
    PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    Begin(O("##LoginCard"), nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);

    ImDrawList* dl = GetWindowDrawList();
    ImVec2 winPos = GetWindowPos();
    
    DrawGradientRect(dl, winPos, ImVec2(winPos.x + cardW, winPos.y + 110), NEON_BLUE, NEON_PURPLE, true);
    dl->AddRectFilled(winPos, ImVec2(winPos.x + cardW, winPos.y + 20), NEON_BLUE, 20.0f, ImDrawFlags_RoundCornersTop);

    SetWindowFontScale(1.4f);
    ImVec2 titleSize = CalcTextSize(L("@DRAGON_GYMAR", "@DRAGON_GYMAR"));
    dl->AddText(ImVec2(winPos.x + (cardW - titleSize.x) * 0.5f, winPos.y + 30), TEXT_WHITE, L("@DRAGON_GYMAR", "@DRAGON_GYMAR"));
    SetWindowFontScale(1.0f);
    
    ImVec2 subSize = CalcTextSize(L("Premium Mod", "ﻢﺳﺮﻟﺍ ﺕﺍﺩﺍﺪﻋﺇ"));
    dl->AddText(ImVec2(winPos.x + (cardW - subSize.x) * 0.5f, winPos.y + 70), NEON_GOLD, L("Premium Mod", "ﻲﺋﺎﻘﻠﺘﻟﺍ ﺐﻌﻠﻟﺍ"));

    SetCursorPosY(130);

    if (!ERROR_MESSAGE.empty()) {
        SetCursorPosX(30);
        PushTextWrapPos(cardW - 30);
        TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", ERROR_MESSAGE.c_str());
        PopTextWrapPos();
        Dummy(ImVec2(0, 15));
    }

    if (is_logging_in) {
        SetCursorPosY(180);
        
        static float spinner_angle = 0.0f;
        spinner_angle += io.DeltaTime * 5.0f;

        float spinner_size = 40.0f;
        ImVec2 spinnerCenter = ImVec2(winPos.x + cardW * 0.5f, winPos.y + 220);

        for (int i = 0; i < 12; i++) {
            float angle = spinner_angle + (i * PI * 2.0f / 12.0f);
            float alpha = (float)(12 - i) / 12.0f;
            ImVec2 dotPos = ImVec2(
                spinnerCenter.x + cosf(angle) * spinner_size,
                spinnerCenter.y + sinf(angle) * spinner_size
            );
            dl->AddCircleFilled(dotPos, 6.0f, NEON_BLUE, (int)(alpha * 255));
        }

        ImVec2 loadingSize = CalcTextSize(L("Authenticating...", "ﻖﻘﺤﺘﻟﺍ ﻱﺭﺎﺟ..."));
        SetCursorPosX((cardW - loadingSize.x) * 0.5f);
        SetCursorPosY(290);
        TextColored(ImVec4(0.6f, 0.6f, 0.65f, 1.0f), L("Authenticating...", "ﻖﻘﺤﺘﻟﺍ ﻱﺭﺎﺟ..."));
    } else {
        SetCursorPosY(160);
        
        ImVec2 infoSize = CalcTextSize(L("Copy your license key and tap login", "ﻝﻮﺧﺩ ﻂﻐﺿﺍ ﻢﺛ ﺺﻴﺧﺮﺘﻟﺍ ﺡﺎﺘﻔﻣ ﺦﺴﻧﺍ"));
        SetCursorPosX((cardW - infoSize.x) * 0.5f);
        TextColored(ImVec4(0.55f, 0.55f, 0.6f, 1.0f), L("Copy your license key and tap login", "ﻝﻮﺧﺩ ﻂﻐﺿﺍ ﻢﺛ ﺺﻴﺧﺮﺘﻟﺍ ﺡﺎﺘﻔﻣ ﺦﺴﻧﺍ"));
        
        Dummy(ImVec2(0, 50));
        
        bool AutoLogin = first_time && !persistent_string["key"].empty();
        
        SetCursorPosX(40);
        PushStyleColor(ImGuiCol_Button,        NEON_BLUE);
        PushStyleColor(ImGuiCol_ButtonHovered, NEON_PURPLE);
        PushStyleColor(ImGuiCol_ButtonActive,  NEON_PINK);
        PushStyleVar(ImGuiStyleVar_FrameRounding, 14.0f);
        
        if (AutoLogin || Button(L("LGNBTN", "ﻝﻮﺧﺩ"), ImVec2(cardW - 80, 65))) {
            if (DEBUG_BYPASS_LOGIN) {
                logged_in = true;
                g_menu.isOpen = true;
            } else {
                JNIEnv* env;
                jint getEnvResult = VM->GetEnv((void**)&env, JNI_VERSION_1_6);
                if (getEnvResult == JNI_EDETACHED) {
                    if (VM->AttachCurrentThread(&env, nullptr) != 0) ERROR_MESSAGE = O("Failed to attach thread to JVM");
                } else if (getEnvResult != JNI_OK) {
                    ERROR_MESSAGE = O("Failed to get JNIEnv");
                } else {
                    std::thread([](std::string androidId, std::string key) {
                        Login(androidId, key);
                    }, getAndroidID(env), AutoLogin ? persistent_string["key"] : getClipboard(env)).detach();
                }
                first_time = false;
            }
        }
        
        PopStyleVar();
        PopStyleColor(3);
        
        Dummy(ImVec2(0, 35));
        
        ImVec2 helpSize = CalcTextSize(L("Your will read", "ﺔﻈﻓﺎﺤﻟﺍ ﻦﻣ ﺡﺎﺘﻔﻤﻟﺍ ﺓﺀﺍﺮﻗ ﻢﺘﻴﺳ"));
        SetCursorPosX((cardW - helpSize.x) * 0.5f);
        TextColored(ImVec4(0.42f, 0.42f, 0.48f, 1.0f), L("Your will read", "ﺔﻈﻓﺎﺤﻟﺍ ﻦﻣ ﺡﺎﺘﻔﻤﻟﺍ ﺓﺀﺍﺮﻗ ﻢﺘﻴﺳ"));
    }

    End();
    PopStyleVar(3);
    PopStyleColor();
    
    End();
}

INLINE void SetupImgui() {
    PACKAGE_NAME = string(getcmdline());

    ImGui::CreateContext();

    auto& style = ImGui::GetStyle();
    auto& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_IsTouchScreen;

    switch_theme(current_theme);

    load_persistence();
    svConfig_Load();
    load_imgui_style();

    static string INI_PATH = O("/data/user_de/0/") + PACKAGE_NAME + O("/no_backup/.ini");
    io.IniFilename = persistent_bool["bImguiAutoSave"] ? INI_PATH.c_str() : nullptr;
    io.ConfigWindowsMoveFromTitleBarOnly = persistent_bool["bMoveOnlyWithTitleBar"];

    ImFontConfig font_cfg;
    font_cfg.SizePixels = persistent_float["fFontScale"];
    io.Fonts->AddFontDefault(&font_cfg);

    // دعم الخط العربي
    static const ImWchar arabic_ranges[] = {
        0x0020, 0x00FF,
        0x0600, 0x06FF,
        0x0750, 0x077F,
        0xFB50, 0xFDFF,
        0xFE70, 0xFEFF,
        0,
    };
    ImFontConfig ar_cfg;
    ar_cfg.MergeMode = true;
    ar_cfg.PixelSnapH = true;
    ar_cfg.SizePixels = font_cfg.SizePixels;
    const char* arabic_fonts[] = {
        "/system/fonts/NotoNaskhArabic-Regular.ttf",
        "/system/fonts/NotoNaskhArabicUI-Regular.ttf",
        "/system/fonts/DroidNaskh-Regular.ttf",
    };
    for (auto path : arabic_fonts) {
        FILE* f = fopen(path, "rb");
        if (f) {
            fclose(f);
            io.Fonts->AddFontFromFileTTF(path, font_cfg.SizePixels, &ar_cfg, arabic_ranges);
            break;
        }
    }

    ImGui_ImplAndroid_Init();
    ImGui_ImplOpenGL3_Init(O("#version 300 es"));

    bImguiSetup = true;
}

DEFINES(EGLBoolean, Draw, EGLDisplay dpy, EGLSurface surface) {
    eglQuerySurface(dpy, surface, EGL_WIDTH, &Width);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &Height);

    if (Width <= 0 || Height <= 0) return _Draw(dpy, surface);

    screenCenter = Vector2(Width / 2, Height / 2);

    if (!bImguiSetup) SetupImgui();

    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame(Width, Height);
    ImGui::NewFrame();

    if (!is_segv_handler_active()) setup_global_segv_handler();
    if (IsExpired()) {
        DrawExpired(io);
    } else if ((!g_Token.empty() && !g_Auth.empty() && g_Token == g_Auth) || DEBUG_BYPASS_LOGIN) {
        DrawFloatingButton(io);
        DrawMenu(io);

{
    SetNextWindowPos(ImVec2(Width * 0.5f, Height - 60.0f), ImGuiCond_Always, ImVec2(0.5f, 1.0f));
    
    Begin(O("##PoweredBy"), nullptr, 
          ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
          ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | 
          ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize | 
          ImGuiWindowFlags_NoInputs);
    
    TextColored(ImColor(155, 48, 255, 255), L("py bower @DRAGON_GYMAR", "@DRAGON_GYMAR"));
    
    End();
}

        if (g_autoPlayCalculating) DrawCalculating(io);
    } else {
        DrawLogin(io);
    }
    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    ImGui_ClearHoverEffect();

    return _Draw(dpy, surface);
}

void __IMGUI__() {
    create_directory_recursive(CONC(O("/data/user_de/0/"), PACKAGE_NAME.c_str(), O("/no_backup")));
}