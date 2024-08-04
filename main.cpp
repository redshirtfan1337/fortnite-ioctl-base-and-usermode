#include <windows.h>
#include <iostream>
#include <d3d9.h>
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx9.h"
#include "ImGui/imgui_impl_win32.h"
#include "driver.h"
#include <Uxtheme.h>
#include <dwmapi.h>
#include "Classes.h"
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "dwmapi.lib")

using namespace std;

uintptr_t dynamic_uworld;
uintptr_t va_text = 0;

IDirect3D9Ex* p_Object = NULL;
IDirect3DDevice9Ex* p_Device = NULL;
D3DPRESENT_PARAMETERS p_Params = { NULL };

HWND GameHwnd = NULL;
HWND MyHwnd = NULL;
MSG Message = { NULL };

uintptr_t procid = NULL;

const char* targetOptions[] = { "Head", "Neck", "Chest" };
static int currentTarget = 0;

static const char* keyNames[] =
{
    (""),
    ("Left Mouse"),
    ("Right Mouse"),
    ("Cancel"),
    ("Middle Mouse"),
    ("Mouse 5"),
    ("Mouse 4"),
    (""),
    ("Backspace"),
    ("Tab"),
    (""),
    (""),
    ("Clear"),
    ("Enter"),
    (""),
    (""),
    ("Shift"),
    ("Control"),
    ("Alt"),
    ("Pause"),
    ("Caps"),
    (""),
    (""),
    (""),
    (""),
    (""),
    (""),
    ("Escape"),
    (""),
    (""),
    (""),
    (""),
    ("Space"),
    ("Page Up"),
    ("Page Down"),
    ("End"),
    ("Home"),
    ("Left"),
    ("Up"),
    ("Right"),
    ("Down"),
    (""),
    (""),
    (""),
    ("Print"),
    ("Insert"),
    ("Delete"),
    (""),
    ("0"),
    ("1"),
    ("2"),
    ("3"),
    ("4"),
    ("5"),
    ("6"),
    ("7"),
    ("8"),
    ("9"),
    (""),
    (""),
    (""),
    (""),
    (""),
    (""),
    (""),
    ("A"),
    ("B"),
    ("C"),
    ("D"),
    ("E"),
    ("F"),
    ("G"),
    ("H"),
    ("I"),
    ("J"),
    ("K"),
    ("L"),
    ("M"),
    ("N"),
    ("O"),
    ("P"),
    ("Q"),
    ("R"),
    ("S"),
    ("T"),
    ("U"),
    ("V"),
    ("W"),
    ("X"),
    ("Y"),
    ("Z"),
    (""),
    (""),
    (""),
    (""),
    (""),
    ("Numpad 0"),
    ("Numpad 1"),
    ("Numpad 2"),
    ("Numpad 3"),
    ("Numpad 4"),
    ("Numpad 5"),
    ("Numpad 6"),
    ("Numpad 7"),
    ("Numpad 8"),
    ("Numpad 9"),
    ("Multiply"),
    ("Add"),
    (""),
    ("Subtract"),
    ("Decimal"),
    ("Divide"),
    ("F1"),
    ("F2"),
    ("F3"),
    ("F4"),
    ("F5"),
    ("F6"),
    ("F7"),
    ("F8"),
    ("F9"),
    ("F10"),
    ("F11"),
    ("F12")
};

static bool Items_ArrayGetter(void* data, int idx, const char** out_text)
{
    const char* const* items = (const char* const*)data;
    if (out_text)
        *out_text = items[idx];
    return true;
}

static int keystatus = 0;

void ChangeKey(void* blank)
{
    keystatus = 1;
    while (true)
    {
        for (int i = 0; i < 0x87; i++)
        {
            if (GetKeyState(i) & 0x8000)
            {
                Features::rAimkey = i;
                keystatus = 0;
                return;
            }
        }
    }
}

void HotkeyButton(int aimkey, void* changekey, int status)
{
    const char* preview_value = NULL;
    if (aimkey >= 0 && aimkey < IM_ARRAYSIZE(keyNames))
        Items_ArrayGetter(keyNames, aimkey, &preview_value);

    string aimkeys;
    if (preview_value == NULL)
        aimkeys = ("Select Key");
    else
        aimkeys = preview_value;

    if (status == 1)
    {
        aimkeys = ("Press Key");
    }
    if (ImGui::Button(aimkeys.c_str(), ImVec2(125, 25)))
    {
        if (status == 0)
        {
            CreateThread(0, 0, (LPTHREAD_START_ROUTINE)changekey, nullptr, 0, nullptr);
        }
    }
}

void AddLine(Vector2 start, Vector2 end, ImU32 color, float thickness)
{
    ImDrawList* drawList = ImGui::GetOverlayDrawList();
    drawList->AddLine(ImVec2(start.x, start.y), ImVec2(end.x, end.y), color, thickness);
}

void FullBox(int X, int Y, int W, int H, const ImColor color, int thickness)
{
    AddLine(Vector2{ (float)X, (float)Y }, Vector2{ (float)(X + W), (float)Y }, color, thickness);
    AddLine(Vector2{ (float)(X + W), (float)Y }, Vector2{ (float)(X + W), (float)(Y + H) }, color, thickness);
    AddLine(Vector2{ (float)X, (float)(Y + H) }, Vector2{ (float)(X + W), (float)(Y + H) }, color, thickness);
    AddLine(Vector2{ (float)X, (float)Y }, Vector2{ (float)X, (float)(Y + H) }, color, thickness);
}

HRESULT DirectXInit() {
    if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &p_Object)))
        exit(3);

    ZeroMemory(&p_Params, sizeof(p_Params));
    p_Params.Windowed = TRUE;
    p_Params.SwapEffect = D3DSWAPEFFECT_DISCARD;
    p_Params.hDeviceWindow = MyHwnd;
    p_Params.MultiSampleQuality = D3DMULTISAMPLE_NONE;
    p_Params.BackBufferFormat = D3DFMT_A8R8G8B8;
    p_Params.BackBufferWidth = ScreenWidth;
    p_Params.BackBufferHeight = ScreenHeight;
    p_Params.EnableAutoDepthStencil = TRUE;
    p_Params.AutoDepthStencilFormat = D3DFMT_D16;
    p_Params.PresentationInterval = 0; // 1 for vsync

    if (FAILED(p_Object->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, MyHwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &p_Params, 0, &p_Device))) {
        p_Object->Release();
        exit(4);
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplWin32_Init(MyHwnd);
    ImGui_ImplDX9_Init(p_Device);
    ImGui::StyleColorsClassic();
    ImGui::GetStyle();
    ImGuiIO& io = ImGui::GetIO();
    ImFont* font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arial.ttf", 12.0f); // change ur font :)

    p_Object->Release();
    return S_OK;
}

void CreateOverlay() {
    WNDCLASSEXA wcex = {
        sizeof(WNDCLASSEXA), 0, DefWindowProcA, 0, 0, nullptr,
        LoadIcon(nullptr, IDI_APPLICATION), LoadCursor(nullptr, IDC_ARROW),
        nullptr, nullptr, ("Fortnite"), LoadIcon(nullptr, IDI_APPLICATION)
    };

    RECT Rect;
    GetWindowRect(GetDesktopWindow(), &Rect);

    RegisterClassExA(&wcex);

    MyHwnd = CreateWindowExA(NULL, ("Fortnite"), ("redshirtfanEdition"), WS_POPUP, Rect.left, Rect.top, Rect.right, Rect.bottom, NULL, NULL, wcex.hInstance, NULL);

    SetWindowLong(MyHwnd, GWL_EXSTYLE, WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_LAYERED);
    MARGINS margin = { -1 };
    DwmExtendFrameIntoClientArea(MyHwnd, &margin);
    ShowWindow(MyHwnd, SW_SHOW);
    SetWindowPos(MyHwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    SetLayeredWindowAttributes(MyHwnd, RGB(0, 0, 0), 255, LWA_ALPHA);
    UpdateWindow(MyHwnd);
}

void CleanupD3D() {
    if (p_Device != NULL) {
        p_Device->EndScene();
        p_Device->Release();
    }
    if (p_Object != NULL) {
        p_Object->Release();
    }
}

bool rMenu = true;
int tab = 0;

void Menu() {
    if (GetAsyncKeyState(VK_INSERT) & 1) rMenu = !rMenu;
    if (rMenu) {
        ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_Once);
        ImGui::SetNextWindowPos(ImVec2((ImGui::GetIO().DisplaySize.x - 600) * 0.5f, (ImGui::GetIO().DisplaySize.y - 400) * 0.5f), ImGuiCond_Once);
        ImGui::Begin("redshirtfan is based", &rMenu, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar);

        ImGui::Text("Overlay");
        ImGui::Separator();

        if (ImGui::BeginTabBar("TabBar")) {
            if (ImGui::BeginTabItem("Aim")) {
                tab = 0;
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Visuals")) {
                tab = 1;
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        if (tab == 0) {
            ImGui::Checkbox("Aimbot", &Features::rAimbot); 
            HotkeyButton(Features::rAimkey, ChangeKey, keystatus);
            ImGui::Checkbox("Fov Circle", &Features::rFovCircle);
            ImGui::SliderInt("Fov Size", &Features::rFovSize, 10, 1000);
            ImGui::SliderInt("Smoothing", &Features::rSmooth, 1, 20);
            ImGui::Combo("Hitbox", &currentTarget, targetOptions, IM_ARRAYSIZE(targetOptions));
        }
        if (tab == 1) {
            ImGui::Checkbox("Fullbox", &Features::rFullbox);
            ImGui::Checkbox("Skeleton", &Features::rSkeleton);
            ImGui::Checkbox("Distance", &Features::rDistance);
            ImGui::Checkbox("Weapon", &Features::rWeapon);
            ImGui::Checkbox("Rank", &Features::rRank);
            ImGui::Checkbox("Render Count", &Features::rRenderCount);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Unload")) {
            exit(0);
        }

        ImGui::End();
    }
}

static float ProjectileSpeed;
static float ProjectileGravity;

Vector3 PredictPlayerPosition(Vector3 CurrentLocation, float Distance, Vector3 Velocity)
{
    Vector3 CalculatedPosition = CurrentLocation;
    if (!ProjectileSpeed) return CalculatedPosition;

    float TimeToTarget = Distance / fabsf(ProjectileSpeed);

    CalculatedPosition.x += Velocity.x * (TimeToTarget) * 120;
    CalculatedPosition.y += Velocity.y * (TimeToTarget) * 120;
    CalculatedPosition.z += Velocity.y * (TimeToTarget) * 120;
    CalculatedPosition.z += fabsf((-49000 / 50) * ProjectileGravity) / 2.0f * (TimeToTarget * 25);

    return CalculatedPosition;
}

__forceinline auto RandomFloat(float a, float b) -> float
{
    float random = ((float)rand()) / (float)RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}

void MoveMouse(float targetX, float targetY) {
    INPUT input;
    input.type = INPUT_MOUSE;
    input.mi.dx = targetX;
    input.mi.dy = targetY;
    input.mi.dwFlags = MOUSEEVENTF_MOVE;
    input.mi.mouseData = 0;
    input.mi.time = 0;
    input.mi.dwExtraInfo = 0;

    SendInput(1, &input, sizeof(INPUT));
}

void aimbot(int bone) {
    if (!Cached::TargetEntity) return;
    auto mesh = Cached::TargetEntity->Mesh();
    if (!mesh) {
        Cached::ClosestDistance = FLT_MAX;
        Cached::TargetEntity = NULL;
        return;
    }
    Vector3 Head3D = mesh->GetSocketLocation(bone);
    Vector2 Head2D = Cached::PlayerController->ProjectWorldLocationToScreen(Head3D);
    ImGuiIO& io = ImGui::GetIO();
    auto root = Cached::TargetEntity->RootComponent();
    Vector3 Velocity = root->GetComponentVelocity();
    float distancee = Head3D.Distance(camera.location) / 100;

    auto distance = GetCrossDistance(Head2D.x, Head2D.y, screenWidth / 2, screenHeight / 2);
    if (distance > Features::rFovSize || Head2D.x == 0 || Head2D.y == 0) {
        Cached::ClosestDistance = FLT_MAX;
        Cached::TargetEntity = NULL;
        return;
    }

    ProjectileSpeed = read<float>((FortPTR)Cached::CurrentWeapon + 0x1CE0);
    ProjectileGravity = read<float>((FortPTR)Cached::CurrentWeapon + 0x1CE4);
    if (ProjectileSpeed == 0 || ProjectileSpeed < 0) ProjectileSpeed = abs(ProjectileSpeed);
    if (ProjectileGravity == 0 || ProjectileGravity < 0) ProjectileGravity = abs(ProjectileGravity);

    float ScreenCenterX = (ImGui::GetIO().DisplaySize.x / 2);
    float ScreenCenterY = (ImGui::GetIO().DisplaySize.y / 2);
    int AimSpeed = Features::rSmooth;
    float TargetX = 0;
    float TargetY = 0;
    float x = Head2D.x;
    float y = Head2D.y;
    Vector2 pred2D;

    if (x != 0)
    {
        if (x > ScreenCenterX)
        {
            TargetX = -(ScreenCenterX - x);
            TargetX /= AimSpeed;
            if (TargetX + ScreenCenterX > ScreenCenterX * 2) TargetX = 0;
        }

        if (x < ScreenCenterX)
        {
            TargetX = x - ScreenCenterX;
            TargetX /= AimSpeed;
            if (TargetX + ScreenCenterX < 0) TargetX = 0;
        }
    }

    if (y != 0)
    {
        if (y > ScreenCenterY)
        {
            TargetY = -(ScreenCenterY - y);
            TargetY /= AimSpeed;
            if (TargetY + ScreenCenterY > ScreenCenterY * 2) TargetY = 0;
        }

        if (y < ScreenCenterY)
        {
            TargetY = y - ScreenCenterY;
            TargetY /= AimSpeed;
            if (TargetY + ScreenCenterY < 0) TargetY = 0;
        }
    }

    MoveMouse(TargetX, TargetY);

    Cached::ClosestDistance = FLT_MAX;
    Cached::TargetEntity = NULL;
}

void ActorLoop() {
    Cached::World = read<UWorld*>(va_text + Offsets::UWorld);
    Uworld_Cam = Cached::World->GetAddress();
    UGameInstance* GameInstance = Cached::World->OwningGameInstance();
    Cached::LocalPlayer = read<ULocalPlayer*>(GameInstance->LocalPlayers());
    Cached::PlayerController = Cached::LocalPlayer->PlayerController();
    Copy_PlayerController_Camera = (FortPTR)Cached::PlayerController;
    Cached::LocalPawn = (AFortPlayerPawn*)Cached::PlayerController->AcknowledgedPawn();
    Cached::LocalPlayerState = Cached::LocalPawn->PlayerState();
    Cached::LocalRootComponent = Cached::LocalPawn->RootComponent();
    Cached::GameState = Cached::World->GameState();
    Cached::CurrentWeapon = Cached::LocalPawn->CurrentWeapon();
    auto PlayerArray = Cached::GameState->PlayerArray();

    std::vector<uint64_t> addressesToRead;
    for (int i = 0; i < PlayerArray.Count; i++) {
        AFortPlayerState* PlayerState = reinterpret_cast<AFortPlayerState*>(PlayerArray.Get(i));
        AFortPlayerPawn* Player = reinterpret_cast<AFortPlayerPawn*>(PlayerState->PawnPrivate());
        USkeletalMeshComponent* Mesh = Player->Mesh();
        if (!Mesh || Player == Cached::LocalPawn) continue;
        addressesToRead.push_back((uintptr_t)Player + 0x758); // despawning 
        addressesToRead.push_back((uintptr_t)PlayerState + 0x9f0); // rank
    }

    auto results = batch_read<char>(addressesToRead);

    size_t index = 0;
    for (int i = 0; i < PlayerArray.Count; i++) {
        AFortPlayerState* PlayerState = reinterpret_cast<AFortPlayerState*>(PlayerArray.Get(i));
        AFortPlayerPawn* Player = reinterpret_cast<AFortPlayerPawn*>(PlayerState->PawnPrivate());
        USkeletalMeshComponent* Mesh = Player->Mesh();
        if (!Mesh || Player == Cached::LocalPawn) continue;

        char despawning = results[index++];
        int32_t RankProgress = *(int32_t*)&results[index++];

        Cached::PlayerState = Player->PlayerState();
        int MyTeamId = Cached::LocalPlayerState->TeamIndex();
        int ActorTeamId = Cached::PlayerState->TeamIndex();

        if (Player == Cached::LocalPawn) continue; // so u dont render urself ingame
        if (MyTeamId && ActorTeamId && MyTeamId == ActorTeamId) continue; // check if valid first since both return 0 in lobby
        if (despawning >> 3) continue; // despawning

        Vector3 Head3D = Mesh->GetSocketLocation(110);
        Vector2 Head2D = Cached::PlayerController->ProjectWorldLocationToScreen(Head3D);
        Vector3 Bottom3D = Mesh->GetSocketLocation(0);
        Vector2 Bottom2D = Cached::PlayerController->ProjectWorldLocationToScreen(Bottom3D);

        float BoxHeight = (float)(Head2D.y - Bottom2D.y);
        float CornerHeight = abs(Head2D.y - Bottom2D.y);
        float CornerWidth = BoxHeight * 0.5;

        float distance = Head3D.Distance(camera.location) / 100;
        if (distance > 300) continue;

        ImGuiIO& io = ImGui::GetIO();
        float screenWidth = io.DisplaySize.x;
        float screenHeight = io.DisplaySize.y;

        Vector2 Center = Head2D;
        Center.y = (Head2D.y + Bottom2D.y) / 2.0f;

        if (Features::rRenderCount) {
            char rendercount[256];
            sprintf(rendercount, "Render Count: %d", PlayerArray.Count);
            ImVec2 text_size = ImGui::CalcTextSize(rendercount);
            float x = (screenWidth - text_size.x) / 2.0f;
            float y = 80.0f;
            ImGui::GetOverlayDrawList()->AddText(ImGui::GetFont(), 15.0f, ImVec2(x, y), ImColor(255, 0, 0, 255), rendercount);
        }
        if (Features::rFovCircle) ImGui::GetOverlayDrawList()->AddCircle(ImVec2(screenWidth / 2, screenHeight / 2), Features::rFovSize, ImColor(255, 255, 255), 64);
        if (Features::rFullbox) FullBox(Center.x - (CornerWidth / 2), Center.y - (CornerHeight / 2), CornerWidth, CornerHeight, ImColor(255, 255, 0), 1.5);
        if (Features::rSkeleton) {
            vector<Vector3> bones = {
                Mesh->GetSocketLocation(66),  // neck 0
                Mesh->GetSocketLocation(9),   // left shoulder 1
                Mesh->GetSocketLocation(10),  // left elbow 2
                Mesh->GetSocketLocation(11),  // left hand 3
                Mesh->GetSocketLocation(38),  // right shoulder 4
                Mesh->GetSocketLocation(39),  // right elbow 5
                Mesh->GetSocketLocation(40),  // right hand 6
                Mesh->GetSocketLocation(2),   // pelvis 7
                Mesh->GetSocketLocation(71),  // left hip 8
                Mesh->GetSocketLocation(72),  // left knee 9
                Mesh->GetSocketLocation(78),  // right hip 10
                Mesh->GetSocketLocation(79),  // right knee 11
                Mesh->GetSocketLocation(75),  // left foot 12
                Mesh->GetSocketLocation(82),  // right foot 13
                Mesh->GetSocketLocation(110)  // head 14
            };
            vector<Vector2> screenPositions(bones.size());
            for (size_t i = 0; i < bones.size(); ++i) {
                screenPositions[i] = Cached::PlayerController->ProjectWorldLocationToScreen(bones[i]);
            }
            AddLine(screenPositions[1], screenPositions[4], ImColor(255, 0, 0), 1.f);   // left shoulder to right shoulder
            AddLine(screenPositions[1], screenPositions[2], ImColor(255, 0, 0), 1.f);   // left shoulder to left elbow
            AddLine(screenPositions[2], screenPositions[3], ImColor(255, 0, 0), 1.f);   // left elbow to left hand
            AddLine(screenPositions[4], screenPositions[5], ImColor(255, 0, 0), 1.f);   // right shoulder to right elbow
            AddLine(screenPositions[5], screenPositions[6], ImColor(255, 0, 0), 1.f);   // right elbow to right hand
            AddLine(screenPositions[0], screenPositions[7], ImColor(255, 0, 0), 1.f);   // neck to pelvis
            AddLine(screenPositions[7], screenPositions[8], ImColor(255, 0, 0), 1.f);   // pelvis to left hip
            AddLine(screenPositions[8], screenPositions[9], ImColor(255, 0, 0), 1.f);   // left hip to left knee
            AddLine(screenPositions[9], screenPositions[12], ImColor(255, 0, 0), 1.f);  // left knee to left foot
            AddLine(screenPositions[7], screenPositions[10], ImColor(255, 0, 0), 1.f);  // pelvis to right hip
            AddLine(screenPositions[10], screenPositions[11], ImColor(255, 0, 0), 1.f); // right hip to right knee
            AddLine(screenPositions[11], screenPositions[13], ImColor(255, 0, 0), 1.f); // right knee to right foot

            float baseRadius = 130.f;
            float radius = baseRadius / distance;
            ImGui::GetOverlayDrawList()->AddCircle(ImVec2(screenPositions[14].x, screenPositions[14].y), radius, ImGui::ColorConvertFloat4ToU32(ImColor(255, 0, 0)), 64, 1);
        }

        float yOffset = Bottom2D.y; 

        if (Features::rWeapon) {
            std::string weaponname = Cached::CurrentWeapon->GetWeaponName((FortPTR)Player);
            if (weaponname.empty()) weaponname = "Hands";

            float textWidth = ImGui::CalcTextSize(weaponname.c_str()).x;
            float xCentered = Bottom2D.x - textWidth / 2;

            ImGui::GetOverlayDrawList()->AddText(
                ImVec2(xCentered, yOffset),
                ImGui::ColorConvertFloat4ToU32(ImColor(255, 0, 0)),
                weaponname.c_str()
            );
            yOffset += ImGui::GetFontSize() + 2;
        }

        if (Features::rRank) {
            std::string rank;
            switch (RankProgress) {
            case 0:  rank = "Bronze 1"; break;
            case 1:  rank = "Bronze 2"; break;
            case 2:  rank = "Bronze 3"; break;
            case 3:  rank = "Silver 1"; break;
            case 4:  rank = "Silver 2"; break;
            case 5:  rank = "Silver 3"; break;
            case 6:  rank = "Gold 1"; break;
            case 7:  rank = "Gold 2"; break;
            case 8:  rank = "Gold 3"; break;
            case 9:  rank = "Platinum 1"; break;
            case 10: rank = "Platinum 2"; break;
            case 11: rank = "Platinum 3"; break;
            case 12: rank = "Diamond 1"; break;
            case 13: rank = "Diamond 2"; break;
            case 14: rank = "Diamond 3"; break;
            case 15: rank = "Elite"; break;
            case 16: rank = "Champion"; break;
            case 17: rank = "Unreal"; break;
            default: rank = "Unranked"; break;
            }

            float textWidth = ImGui::CalcTextSize(rank.c_str()).x;
            float xCentered = Bottom2D.x - textWidth / 2;

            ImGui::GetOverlayDrawList()->AddText(
                ImVec2(xCentered, yOffset),
                ImGui::ColorConvertFloat4ToU32(ImColor(255, 255, 0)),
                rank.c_str()
            );
            yOffset += ImGui::GetFontSize() + 2;
        }

        if (distance) {
            std::string distanceStr = std::to_string(distance);

            float textWidth = ImGui::CalcTextSize(distanceStr.c_str()).x;
            float xCentered = Bottom2D.x - textWidth / 2;

            ImGui::GetOverlayDrawList()->AddText(
                ImVec2(xCentered, yOffset),
                ImGui::ColorConvertFloat4ToU32(ImColor(255, 0, 0)),
                distanceStr.c_str()
            );
        }

        auto dist = GetCrossDistance(Head2D.x, Head2D.y, screenWidth / 2, screenHeight / 2);
        if (dist < Features::rFovSize && dist < Cached::ClosestDistance) {
            Cached::ClosestDistance = dist;
            Cached::TargetEntity = Player;
        }
    }
}

void Render() {
    ActorLoop();
}

WPARAM MainLoop() {
    static RECT old_rc;
    ZeroMemory(&Message, sizeof(MSG));

    while (Message.message != WM_QUIT) {
        if (PeekMessage(&Message, MyHwnd, 0, 0, PM_REMOVE)) {
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }

        HWND hwnd_active = GetForegroundWindow();
        if (hwnd_active == GameHwnd) {
            HWND hwndtest = GetWindow(hwnd_active, GW_HWNDPREV);
            SetWindowPos(MyHwnd, hwndtest, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        }

        RECT rc;
        POINT xy;
        ZeroMemory(&rc, sizeof(RECT));
        ZeroMemory(&xy, sizeof(POINT));
        GetClientRect(GameHwnd, &rc);
        ClientToScreen(GameHwnd, &xy);
        rc.left = xy.x;
        rc.top = xy.y;

        ImGuiIO& io = ImGui::GetIO();
        io.ImeWindowHandle = GameHwnd;
        io.DeltaTime = 1.0f / 60.0f;

        POINT p;
        GetCursorPos(&p);
        io.MousePos.x = p.x - xy.x;
        io.MousePos.y = p.y - xy.y;

        if (GetAsyncKeyState(0x1)) {
            io.MouseDown[0] = true;
            io.MouseClicked[0] = true;
            io.MouseClickedPos[0].x = io.MousePos.x;
            io.MouseClickedPos[0].x = io.MousePos.y;
        }
        else {
            io.MouseDown[0] = false;
        }

        if (rc.left != old_rc.left || rc.right != old_rc.right || rc.top != old_rc.top || rc.bottom != old_rc.bottom) {
            old_rc = rc;

            ScreenWidth = rc.right;
            ScreenHeight = rc.bottom;

            p_Params.BackBufferWidth = ScreenWidth;
            p_Params.BackBufferHeight = ScreenHeight;
            SetWindowPos(MyHwnd, (HWND)0, xy.x, xy.y, ScreenWidth, ScreenHeight, SWP_NOREDRAW);
            p_Device->Reset(&p_Params);
        }

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        Render();
        if (GetAsyncKeyState(Features::rAimkey) && Features::rAimbot) {
            if (currentTarget == 0)
                aimbot(110);
            if (currentTarget == 1)
                aimbot(67);
            if (currentTarget == 2)
                aimbot(37);
        }
        Menu();

        ImGui::EndFrame();
        p_Device->SetRenderState(D3DRS_ZENABLE, false);
        p_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
        p_Device->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
        p_Device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
        if (p_Device->BeginScene() >= 0) {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            p_Device->EndScene();
        }

        HRESULT result = p_Device->Present(NULL, NULL, NULL, NULL);

        if (result == D3DERR_DEVICELOST && p_Device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) {
            ImGui_ImplDX9_InvalidateDeviceObjects();
            p_Device->Reset(&p_Params);
            ImGui_ImplDX9_CreateDeviceObjects();
        }
    }
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupD3D();
    DestroyWindow(MyHwnd);

    return Message.wParam;
}

int main() {
    if (!mem::find_driver()) cout << "ERROR: Driver not found (cheat cannot perform without)" << endl;
    mem::process_id = mem::find_process("FortniteClient-Win64-Shipping.exe");

    virtualaddy = mem::find_image();
    cr3 = mem::fetch_cr3();
    for (int i = 0; i < 25; i++) 
        if ((read<__int32>(virtualaddy + (i * 0x1000) + 0x250)) == 0x260E020B) {
            va_text = virtualaddy + ((i + 1) * 0x1000);
        }

    cout << "Base Address -> " << virtualaddy << endl;
    cout << "CR3 -> " << cr3 << endl;
    cout << "VAText -> " << va_text << endl;

    CreateOverlay();
    DirectXInit();
    MainLoop();
}
