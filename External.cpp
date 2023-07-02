#include <dwmapi.h>
#include <vector>
#include <Windows.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <thread>
#include <Windows.h>
#include <direct.h>
#include <string>
#include <intrin.h>
#include <list>
#include <WinUser.h>
#include <Windows.h>
#include <filesystem>
#include <random>
#include <TlHelp32.h>
#include "Driver/lib.hpp"
#include "Driver/Driver.h"
#include "External.h"
#include "settings.h"
#include "D3DX/d3dx9math.h"
#include "d3d9_x.h"

RECT GameRect = { NULL };
D3DPRESENT_PARAMETERS d3dpp;
IDirect3D9Ex* p_Object = NULL;
LPDIRECT3DDEVICE9 D3dDevice = NULL;
LPDIRECT3DVERTEXBUFFER9 TriBuf = NULL;
MSG Message = { NULL };
MARGINS Margin = { -1 };
ImFont* fn = nullptr;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int center_x = GetSystemMetrics(0) / 2 - 3;
int center_y = GetSystemMetrics(1) / 2 - 3;

namespace game
{

	namespace globals
	{
		uintptr_t GWorld;
		uintptr_t UworldPointer;
		uintptr_t LocalPawn;
		uintptr_t PlayerState;
		uintptr_t Localplayer;
		uintptr_t RootComponent;
		uintptr_t PlayerController;
		uintptr_t Persistentlevel;
		uintptr_t Gameinstance;
		uintptr_t LocalPlayers;
		Vector3 relativelocation;
		uintptr_t AActors;
		int ActorCount;
	};

	struct actor
	{
		uint64_t pawn;
		uintptr_t mesh;
		uintptr_t enemy_state;
		INT32 team_id;
		float DBNO;
		bool is_valid;
		ImU32 color;
	};

	inline bool operator==(const actor& a, const actor& b) {
		if (a.pawn == b.pawn)
			return true;
		return false;
	}

}
std::vector<game::actor> playerlist;

namespace FortCamera
{
	Vector3 Location;
	Vector3 Rotation;
	float FieldOfView;
}

D3DXMATRIX Matrix(Vector3 rot, Vector3 origin = Vector3(0, 0, 0))
{
	float radPitch = (rot.x * float(M_PI) / 180.f);
	float radYaw = (rot.y * float(M_PI) / 180.f);
	float radRoll = (rot.z * float(M_PI) / 180.f);

	float SP = sinf(radPitch);
	float CP = cosf(radPitch);
	float SY = sinf(radYaw);
	float CY = cosf(radYaw);
	float SR = sinf(radRoll);
	float CR = cosf(radRoll);

	D3DMATRIX matrix;
	matrix.m[0][0] = CP * CY;
	matrix.m[0][1] = CP * SY;
	matrix.m[0][2] = SP;
	matrix.m[0][3] = 0.f;

	matrix.m[1][0] = SR * SP * CY - CR * SY;
	matrix.m[1][1] = SR * SP * SY + CR * CY;
	matrix.m[1][2] = -SR * CP;
	matrix.m[1][3] = 0.f;

	matrix.m[2][0] = -(CR * SP * CY + SR * SY);
	matrix.m[2][1] = CY * SR - CR * SP * SY;
	matrix.m[2][2] = CR * CP;
	matrix.m[2][3] = 0.f;

	matrix.m[3][0] = origin.x;
	matrix.m[3][1] = origin.y;
	matrix.m[3][2] = origin.z;
	matrix.m[3][3] = 1.f;

	return matrix;
}

D3DMATRIX matrixx(D3DMATRIX pM1, D3DMATRIX pM2)
{
	D3DMATRIX pOut;
	pOut._11 = pM1._11 * pM2._11 + pM1._12 * pM2._21 + pM1._13 * pM2._31 + pM1._14 * pM2._41;
	pOut._12 = pM1._11 * pM2._12 + pM1._12 * pM2._22 + pM1._13 * pM2._32 + pM1._14 * pM2._42;
	pOut._13 = pM1._11 * pM2._13 + pM1._12 * pM2._23 + pM1._13 * pM2._33 + pM1._14 * pM2._43;
	pOut._14 = pM1._11 * pM2._14 + pM1._12 * pM2._24 + pM1._13 * pM2._34 + pM1._14 * pM2._44;
	pOut._21 = pM1._21 * pM2._11 + pM1._22 * pM2._21 + pM1._23 * pM2._31 + pM1._24 * pM2._41;
	pOut._22 = pM1._21 * pM2._12 + pM1._22 * pM2._22 + pM1._23 * pM2._32 + pM1._24 * pM2._42;
	pOut._23 = pM1._21 * pM2._13 + pM1._22 * pM2._23 + pM1._23 * pM2._33 + pM1._24 * pM2._43;
	pOut._24 = pM1._21 * pM2._14 + pM1._22 * pM2._24 + pM1._23 * pM2._34 + pM1._24 * pM2._44;
	pOut._31 = pM1._31 * pM2._11 + pM1._32 * pM2._21 + pM1._33 * pM2._31 + pM1._34 * pM2._41;
	pOut._32 = pM1._31 * pM2._12 + pM1._32 * pM2._22 + pM1._33 * pM2._32 + pM1._34 * pM2._42;
	pOut._33 = pM1._31 * pM2._13 + pM1._32 * pM2._23 + pM1._33 * pM2._33 + pM1._34 * pM2._43;
	pOut._34 = pM1._31 * pM2._14 + pM1._32 * pM2._24 + pM1._33 * pM2._34 + pM1._34 * pM2._44;
	pOut._41 = pM1._41 * pM2._11 + pM1._42 * pM2._21 + pM1._43 * pM2._31 + pM1._44 * pM2._41;
	pOut._42 = pM1._41 * pM2._12 + pM1._42 * pM2._22 + pM1._43 * pM2._32 + pM1._44 * pM2._42;
	pOut._43 = pM1._41 * pM2._13 + pM1._42 * pM2._23 + pM1._43 * pM2._33 + pM1._44 * pM2._43;
	pOut._44 = pM1._41 * pM2._14 + pM1._42 * pM2._24 + pM1._43 * pM2._34 + pM1._44 * pM2._44;

	return pOut;
}

bool IsVec3Valid(Vector3 vec3)
{
	return !(vec3.x == 0 && vec3.y == 0 && vec3.z == 0);
}

bool IsInScreen(Vector3 pos, int over = 30) {
	if (pos.x > -over && pos.x < Width + over && pos.y > -over && pos.y < Height + over)
		return true;
	return false;
}

float powf_(float _X, float _Y) {
	return (_mm_cvtss_f32(_mm_pow_ps(_mm_set_ss(_X), _mm_set_ss(_Y))));
}

double GetDistance(double x1, double y1, double z1, double x2, double y2) {
	return sqrtf(powf((x2 - x1), 2) + powf_((y2 - y1), 2));
}

boolean in_rect(double centerX, double centerY, double radius, double x, double y) {
	return x >= centerX - radius && x <= centerX + radius &&
		y >= centerY - radius && y <= centerY + radius;
}

Vector3 BoneArray(uintptr_t mesh, int id)
{
	__int64 rdx, rcx;
	rdx = Drv.read<int>(mesh + 0x6D0);
	rcx = Drv.read<__int64>(mesh + 0x688 + (rdx * 0x10));
	FTransform bone = Drv.read<FTransform>(rcx + (0x30 * id));
	FTransform ComponentToWorld = Drv.read<FTransform>(mesh + 0x180);

	D3DMATRIX Matrix;
	Matrix = matrixx(bone.ToMatrixWithScale(), ComponentToWorld.ToMatrixWithScale());

	return Vector3(Matrix._41, Matrix._42, Matrix._43);
}

bool isVisible(uint64_t mesh)
{
	float tik = Drv.read<float>(mesh + 0x358);
	float tok = Drv.read<float>(mesh + 0x360);
	const float tick = 0.06f;
	return tok + tick >= tik;
}

void GetCamera(uintptr_t LocalRoot)
{
	int64 v1;
	int64 v6;
	int64 v7;
	int64 v8;

	v1 = Drv.read<int64>(game::globals::Localplayer + 0xC8);
	int64 v9 = Drv.read<int64>(v1 + 0x8);

	FortCamera::FieldOfView = 80.0f / (Drv.read<double>(v9 + 0x7F0) / 1.19f);

	FortCamera::Rotation.x = Drv.read<float>(v9 + 0x6a8);
	FortCamera::Rotation.y = Drv.read<float>(LocalRoot + 0x16C);

	FortCamera::Location = Drv.read<Vector3>(Drv.read<uint64_t>(game::globals::GWorld + 0x120));

	return;
}

Vector3 ProjectWorldToScreen(Vector3 WorldLocation)
{
	FortCamera::Rotation.x = (asin(FortCamera::Rotation.x)) * (180.0 / M_PI);

	D3DMATRIX tempMatrix = Matrix(FortCamera::Rotation);

	Vector3 vAxisX = Vector3(tempMatrix.m[0][0], tempMatrix.m[0][1], tempMatrix.m[0][2]);
	Vector3 vAxisY = Vector3(tempMatrix.m[1][0], tempMatrix.m[1][1], tempMatrix.m[1][2]);
	Vector3 vAxisZ = Vector3(tempMatrix.m[2][0], tempMatrix.m[2][1], tempMatrix.m[2][2]);

	Vector3 vDelta = WorldLocation - FortCamera::Location;
	Vector3 vTransformed = Vector3(vDelta.Dot(vAxisY), vDelta.Dot(vAxisZ), vDelta.Dot(vAxisX));

	if (vTransformed.z < 1.f)
		vTransformed.z = 1.f;

	return Vector3((Width / 2.0f) + vTransformed.x * (((Width / 2.0f) / tanf(FortCamera::FieldOfView * (float)M_PI / 360.f))) / vTransformed.z, (Height / 2.0f) - vTransformed.y * (((Width / 2.0f) / tanf(FortCamera::FieldOfView * (float)M_PI / 360.f))) / vTransformed.z, 0);
}

uintptr_t GetPid(const wchar_t* processName)
{
	PROCESSENTRY32 procEntry32;
	uintptr_t pID = 0;

	procEntry32.dwSize = sizeof(PROCESSENTRY32);

	HANDLE hProcSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (!hProcSnap || hProcSnap == INVALID_HANDLE_VALUE)
		return 0;

	while (Process32Next(hProcSnap, &procEntry32))
	{
		if (!wcscmp(processName, procEntry32.szExeFile))
		{
			pID = procEntry32.th32ProcessID;

			CloseHandle(hProcSnap);
		}
	}

	CloseHandle(hProcSnap);
	return pID;
}

void SetWindowToTarget()
{
	while (true)
	{
		if (cheat::hWnd)
		{
			ZeroMemory(&GameRect, sizeof(GameRect));
			GetWindowRect(cheat::hWnd, &GameRect);
			Width = GameRect.right - GameRect.left;
			Height = GameRect.bottom - GameRect.top;
			DWORD dwStyle = GetWindowLong(cheat::hWnd, GWL_STYLE);

			if (dwStyle & WS_BORDER)
			{
				GameRect.top += 32;
				Height -= 39;
			}

			MoveWindow(Window, GameRect.left, GameRect.top, Width, Height, true);
		}
		else
		{
			exit(0);
		}
	}
}

void xInitD3d()
{
	if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &p_Object)))
		exit(3);

	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.BackBufferWidth = Width;
	d3dpp.BackBufferHeight = Height;
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
	d3dpp.MultiSampleQuality = D3DMULTISAMPLE_NONE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.hDeviceWindow = Window;
	d3dpp.Windowed = TRUE;

	p_Object->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, Window, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &D3dDevice);

	IMGUI_CHECKVERSION();

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;

	static const ImWchar icons_ranges[] = { 0xf000, 0xf3ff, 0 };
	ImFontConfig icons_config;

	icons_config.MergeMode = true;
	icons_config.PixelSnapH = true;
	icons_config.OversampleH = 3;
	icons_config.OversampleV = 3;

	fn = io.Fonts->AddFontFromMemoryTTF(&fortnite, sizeof fortnite, 22, NULL, io.Fonts->GetGlyphRangesCyrillic());

	ImGui_ImplWin32_Init(Window);
	ImGui_ImplDX9_Init(D3dDevice);

	ImGui::StyleColorsClassic();
}

float RandomFloat(float a, float b)
{
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}

void aimbot(float x, float y)
{
	float ScreenCenterX = (Width / 2);
	float ScreenCenterY = (Height / 2);
	int AimSpeed = smooth;
	float TargetX = 0;
	float TargetY = 0;

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

	// Removed humanized mouse aimbot
	mouse_event(MOUSEEVENTF_MOVE, TargetX, TargetY, NULL, NULL);

	return;
}

void skeleton_esp(uintptr_t mesh, ImU32 color)
{
	Vector3 head, neck, penis, Hip, Right_Hip, Left_Hip, Right_Knee, Left_Knee, Right_Foot, Left_Foot, Left_Shoulder, Right_Shoulder, Right_Elbow, Left_Elbow, Left_Hand, Right_Hand;
	head = ProjectWorldToScreen(BoneArray(mesh, 66));
	neck = ProjectWorldToScreen(BoneArray(mesh, 64));
	penis = ProjectWorldToScreen(BoneArray(mesh, 5));
	Hip = ProjectWorldToScreen(BoneArray(mesh, 2));
	Right_Hip = ProjectWorldToScreen(BoneArray(mesh, 74));
	Left_Hip = ProjectWorldToScreen(BoneArray(mesh, 67));
	Right_Knee = ProjectWorldToScreen(BoneArray(mesh, 75));
	Left_Knee = ProjectWorldToScreen(BoneArray(mesh, 68));
	Right_Foot = ProjectWorldToScreen(BoneArray(mesh, 83));
	Left_Foot = ProjectWorldToScreen(BoneArray(mesh, 82));
	Left_Shoulder = ProjectWorldToScreen(BoneArray(mesh, 9));
	Right_Shoulder = ProjectWorldToScreen(BoneArray(mesh, 37));
	Right_Elbow = ProjectWorldToScreen(BoneArray(mesh, 38));
	Left_Elbow = ProjectWorldToScreen(BoneArray(mesh, 10));
	Left_Hand = ProjectWorldToScreen(BoneArray(mesh, 32));
	Right_Hand = ProjectWorldToScreen(BoneArray(mesh, 60));

	ImGui::GetOverlayDrawList()->AddLine(ImVec2(head.x, head.y), ImVec2(neck.x, neck.y), color, 0.6f);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(neck.x, neck.y), ImVec2(penis.x, penis.y), color, 0.6f);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(penis.x, penis.y), ImVec2(Hip.x, Hip.y), color, 0.6f);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(Hip.x, Hip.y), ImVec2(Right_Hip.x, Right_Hip.y), color, 0.6f);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(Hip.x, Hip.y), ImVec2(Left_Hip.x, Left_Hip.y), color, 0.6f);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(Right_Hip.x, Right_Hip.y), ImVec2(Right_Knee.x, Right_Knee.y), color, 0.6f);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(Left_Hip.x, Left_Hip.y), ImVec2(Left_Knee.x, Left_Knee.y), color, 0.6f);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(Right_Knee.x, Right_Knee.y), ImVec2(Right_Foot.x, Right_Foot.y), color, 0.6f);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(Left_Knee.x, Left_Knee.y), ImVec2(Left_Foot.x, Left_Foot.y), color, 0.6f);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(neck.x, neck.y), ImVec2(Left_Shoulder.x, Left_Shoulder.y), color, 0.6f);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(neck.x, neck.y), ImVec2(Right_Shoulder.x, Right_Shoulder.y), color, 0.6f);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(Left_Shoulder.x, Left_Shoulder.y), ImVec2(Left_Elbow.x, Left_Elbow.y), color, 0.6f);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(Right_Shoulder.x, Right_Shoulder.y), ImVec2(Right_Elbow.x, Right_Elbow.y), color, 0.6f);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(Left_Elbow.x, Left_Elbow.y), ImVec2(Left_Hand.x, Left_Hand.y), color, 0.6f);
	ImGui::GetOverlayDrawList()->AddLine(ImVec2(Right_Elbow.x, Right_Elbow.y), ImVec2(Right_Hand.x, Right_Hand.y), color, 0.6f);
}

void draw_rect(int x, int y, int w, int h, ImColor color, int thickness) {
	ImGui::GetOverlayDrawList()->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), color, 0.f, 0, thickness);
}

void DrawFortniteText(ImVec2 pos, ImU32 color, const char* str)
{
	std::string utf_8_1 = std::string(str);
	std::string utf_8_2 = string_To_UTF8(utf_8_1);
	ImGui::GetOverlayDrawList()->AddText(pos, ImGui::GetColorU32(color), utf_8_2.c_str());
}

void corner_esp(float OffsetW, float OffsetH, int X, int Y, int W, int H, ImU32 Color, int thickness) {
	ImDrawList* p = ImGui::GetOverlayDrawList();

	float lineW = (W / OffsetW);
	float lineH = (H / OffsetH);

	p->AddLine(ImVec2(X, Y), ImVec2(X, Y + lineH), ImGui::GetColorU32(Color), thickness);
	p->AddLine(ImVec2(X, Y), ImVec2(X + lineW, Y), ImGui::GetColorU32(Color), thickness);
	p->AddLine(ImVec2(X + W - lineW, Y), ImVec2(X + W, Y), ImGui::GetColorU32(Color), thickness);
	p->AddLine(ImVec2(X + W, Y), ImVec2(X + W, Y + lineH), ImGui::GetColorU32(Color), thickness);
	p->AddLine(ImVec2(X, Y + H - lineH), ImVec2(X, Y + H), ImGui::GetColorU32(Color), thickness);
	p->AddLine(ImVec2(X, Y + H), ImVec2(X + lineW, Y + H), ImGui::GetColorU32(Color), thickness);
	p->AddLine(ImVec2(X + W - lineW, Y + H), ImVec2(X + W, Y + H), ImGui::GetColorU32(Color), thickness);
	p->AddLine(ImVec2(X + W, Y + H - lineH), ImVec2(X + W, Y + H), ImGui::GetColorU32(Color), thickness);
}

void blackballs()
{
	float closestDistance = FLT_MAX;
	int closestPawn = 1337;

	ImGui::GetOverlayDrawList()->AddCircle(ImVec2(center_x, center_y), AimFOV * FortCamera::FieldOfView, ImColor(255, 255, 255, 255), 2000, 2.0f);

	for (int x = 0; x < playerlist.size(); x++)
	{
		game::actor entity = playerlist[x];

		Vector3 vHeadBone = BoneArray(entity.mesh, 66);
		if (!IsVec3Valid(vHeadBone))continue;

		Vector3 vRootBone = BoneArray(entity.mesh, 0);
		if (!IsVec3Valid(vRootBone))continue;

		Vector3 vTop = Vector3(vHeadBone.x, vHeadBone.y, vHeadBone.z + 22);
		if (!IsVec3Valid(vTop))continue;

		Vector3 vBottom = Vector3(vRootBone.x, vRootBone.y, vRootBone.z - 5);
		if (!IsVec3Valid(vBottom))continue;

		Vector3 vHeadBoneOut = ProjectWorldToScreen(vHeadBone);
		if (!IsVec3Valid(vHeadBoneOut))continue;

		Vector3 vRootBoneOut = ProjectWorldToScreen(vRootBone);
		if (!IsVec3Valid(vRootBoneOut))continue;

		Vector3 vTopBoneOut = ProjectWorldToScreen(vTop);
		if (!IsVec3Valid(vTopBoneOut))continue;

		Vector3 vBottomBoneOut = ProjectWorldToScreen(vBottom);
		if (!IsVec3Valid(vBottomBoneOut))continue;

		if (entity.team_id == Drv.read<char>(entity.enemy_state + 0xf60)) continue;
		if (entity.pawn == game::globals::LocalPawn) continue;

		bool isDBNO = (Drv.read<BYTE>(entity.pawn + 0x8c0) & 0x10);
		if (!isDBNO) continue;

		auto dx = vHeadBoneOut.x - center_x;
		auto dy = vHeadBoneOut.y - center_y;
		auto dist = sqrtf(dx * dx + dy * dy);
		if (dist < closestDistance)
		{
			closestDistance = dist;
			closestPawn = x;
		}

		float player_distance = game::globals::relativelocation.Distance(vRootBoneOut) / 100.f;
		if (entity.team_id != Drv.read<char>(entity.enemy_state + 0xf60) && IsInScreen(vRootBoneOut))
		{

			if (distance)
			{
				char text_distance[256];
				float BoxHeight = vBottomBoneOut.y - vTopBoneOut.y;
				float BoxWidth = BoxHeight / 1.9f;
				sprintf_s(text_distance, "(%.fm)", player_distance);
				ImVec2 text_size = ImGui::CalcTextSize(text_distance);
				DrawFortniteText(ImVec2(vBottomBoneOut.x - (BoxWidth / 2), vTopBoneOut.y - (text_size.x / 2)), ImColor(255, 255, 255, 255), text_distance); // - (text_size.x / 2)
			}

			if (Snaplines) {
				ImGui::GetOverlayDrawList()->AddLine(ImVec2(Width / 2, Height), ImVec2(vRootBoneOut.x, vRootBoneOut.y), entity.color, 0.5);
			}

			if (box_type == SELECT_BOXES_NORMAL) {
				float BoxHeight = vBottomBoneOut.y - vTopBoneOut.y;
				float BoxWidth = BoxHeight / 1.9f;
				draw_rect(vBottomBoneOut.x - (BoxWidth / 2), vTopBoneOut.y,
					BoxWidth, BoxWidth, entity.color, 2);
			}
			else if (box_type == SELECT_BOXES_CORNER)
			{
				float BoxHeight = vBottomBoneOut.y - vTopBoneOut.y;
				float BoxWidth = BoxHeight / 1.9f;
				corner_esp(4, 4, vBottomBoneOut.x - (BoxWidth / 2), vTopBoneOut.y,
					BoxWidth, BoxHeight, entity.color, 1.f);
			}
			else if (box_type == SELECT_BOXES_THREED)
			{
				Vector3 bottom1 = ProjectWorldToScreen(Vector3(vRootBone.x + 40, vRootBone.y - 40, vRootBone.z));
				Vector3 bottom2 = ProjectWorldToScreen(Vector3(vRootBone.x - 40, vRootBone.y - 40, vRootBone.z));
				Vector3 bottom3 = ProjectWorldToScreen(Vector3(vRootBone.x - 40, vRootBone.y + 40, vRootBone.z));
				Vector3 bottom4 = ProjectWorldToScreen(Vector3(vRootBone.x + 40, vRootBone.y + 40, vRootBone.z));

				Vector3 top1 = ProjectWorldToScreen(Vector3(vHeadBone.x + 40, vHeadBone.y - 40, vHeadBone.z + 15));
				Vector3 top2 = ProjectWorldToScreen(Vector3(vHeadBone.x - 40, vHeadBone.y - 40, vHeadBone.z + 15));
				Vector3 top3 = ProjectWorldToScreen(Vector3(vHeadBone.x - 40, vHeadBone.y + 40, vHeadBone.z + 15));
				Vector3 top4 = ProjectWorldToScreen(Vector3(vHeadBone.x + 40, vHeadBone.y + 40, vHeadBone.z + 15));

				ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom1.x, bottom1.y), ImVec2(top1.x, top1.y), entity.color, 2.0f);
				ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom2.x, bottom2.y), ImVec2(top2.x, top2.y), entity.color, 2.0f);
				ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom3.x, bottom3.y), ImVec2(top3.x, top3.y), entity.color, 2.0f);
				ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom4.x, bottom4.y), ImVec2(top4.x, top4.y), entity.color, 2.0f);

				ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom1.x, bottom1.y), ImVec2(bottom2.x, bottom2.y), entity.color, 2.0f);
				ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom2.x, bottom2.y), ImVec2(bottom3.x, bottom3.y), entity.color, 2.0f);
				ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom3.x, bottom3.y), ImVec2(bottom4.x, bottom4.y), entity.color, 2.0f);
				ImGui::GetOverlayDrawList()->AddLine(ImVec2(bottom4.x, bottom4.y), ImVec2(bottom1.x, bottom1.y), entity.color, 2.0f);

				ImGui::GetOverlayDrawList()->AddLine(ImVec2(top1.x, top1.y), ImVec2(top2.x, top2.y), entity.color, 2.0f);
				ImGui::GetOverlayDrawList()->AddLine(ImVec2(top2.x, top2.y), ImVec2(top3.x, top3.y), entity.color, 2.0f);
				ImGui::GetOverlayDrawList()->AddLine(ImVec2(top3.x, top3.y), ImVec2(top4.x, top4.y), entity.color, 2.0f);
				ImGui::GetOverlayDrawList()->AddLine(ImVec2(top4.x, top4.y), ImVec2(top1.x, top1.y), entity.color, 2.0f);
			}

			if (bhead)
			{
				Vector3 BottomOne = Vector3(vHeadBone.x + 10, vHeadBone.y + 10, vHeadBone.z - 10); Vector3 BottomOneW2S = ProjectWorldToScreen(BottomOne);
				Vector3 BottomTwo = Vector3(vHeadBone.x - 10, vHeadBone.y - 10, vHeadBone.z - 10); Vector3 BottomTwoW2S = ProjectWorldToScreen(BottomTwo);
				Vector3 BottomThree = Vector3(vHeadBone.x + 10, vHeadBone.y - 10, vHeadBone.z - 10); Vector3 BottomThreeW2S = ProjectWorldToScreen(BottomThree);
				Vector3 BottomFour = Vector3(vHeadBone.x - 10, vHeadBone.y + 10, vHeadBone.z - 10); Vector3 BottomFourW2S = ProjectWorldToScreen(BottomFour);
				Vector3 TopOne = Vector3(vHeadBone.x + 10, vHeadBone.y + 10, vHeadBone.z + 10); Vector3 TopOneW2S = ProjectWorldToScreen(TopOne);
				Vector3 TopTwo = Vector3(vHeadBone.x - 10, vHeadBone.y - 10, vHeadBone.z + 10); Vector3 TopTwoW2S = ProjectWorldToScreen(TopTwo);
				Vector3 TopThree = Vector3(vHeadBone.x + 10, vHeadBone.y - 10, vHeadBone.z + 10); Vector3 TopThreeW2S = ProjectWorldToScreen(TopThree);
				Vector3 TopFour = Vector3(vHeadBone.x - 10, vHeadBone.y + 10, vHeadBone.z + 10); Vector3 TopFourW2S = ProjectWorldToScreen(TopFour);

				ImU32 red = ImColor(255, 0, 0, 255);

				ImGui::GetOverlayDrawList()->AddLine(ImVec2(BottomOneW2S.x, BottomOneW2S.y), ImVec2(BottomThreeW2S.x, BottomThreeW2S.y), red, 1.f);
				ImGui::GetOverlayDrawList()->AddLine(ImVec2(BottomThreeW2S.x, BottomThreeW2S.y), ImVec2(BottomTwoW2S.x, BottomTwoW2S.y), red, 1.f);
				ImGui::GetOverlayDrawList()->AddLine(ImVec2(BottomTwoW2S.x, BottomTwoW2S.y), ImVec2(BottomFourW2S.x, BottomFourW2S.y), red, 1.f);
				ImGui::GetOverlayDrawList()->AddLine(ImVec2(BottomFourW2S.x, BottomFourW2S.y), ImVec2(BottomOneW2S.x, BottomOneW2S.y), red, 1.f);
				ImGui::GetOverlayDrawList()->AddLine(ImVec2(TopOneW2S.x, TopOneW2S.y), ImVec2(TopThreeW2S.x, TopThreeW2S.y), red, 1.f);
				ImGui::GetOverlayDrawList()->AddLine(ImVec2(TopThreeW2S.x, TopThreeW2S.y), ImVec2(TopTwoW2S.x, TopTwoW2S.y), red, 1.f);
				ImGui::GetOverlayDrawList()->AddLine(ImVec2(TopTwoW2S.x, TopTwoW2S.y), ImVec2(TopFourW2S.x, TopFourW2S.y), red, 1.f);
				ImGui::GetOverlayDrawList()->AddLine(ImVec2(TopFourW2S.x, TopFourW2S.y), ImVec2(TopOneW2S.x, TopOneW2S.y), red, 1.f);
				ImGui::GetOverlayDrawList()->AddLine(ImVec2(BottomOneW2S.x, BottomOneW2S.y), ImVec2(TopOneW2S.x, TopOneW2S.y), red, 1.f);
				ImGui::GetOverlayDrawList()->AddLine(ImVec2(BottomTwoW2S.x, BottomTwoW2S.y), ImVec2(TopTwoW2S.x, TopTwoW2S.y), red, 1.f);
				ImGui::GetOverlayDrawList()->AddLine(ImVec2(BottomThreeW2S.x, BottomThreeW2S.y), ImVec2(TopThreeW2S.x, TopThreeW2S.y), red, 1.f);
				ImGui::GetOverlayDrawList()->AddLine(ImVec2(BottomFourW2S.x, BottomFourW2S.y), ImVec2(TopFourW2S.x, TopFourW2S.y), red, 1.f);
			}

			if (skell)
			{
				skeleton_esp(entity.mesh, entity.color);
			}

		}

	}

	if (Aimbot && closestPawn != 1337 && game::globals::LocalPawn)
	{
		game::actor player = playerlist[closestPawn];

		bool Copy_Aimbot = Aimbot;
		bool Copy_Triggerbot = autofire;
		bool Copy_lines = TargetLines;
		float Copy_Aim_Fieldofview = AimFOV;
		bool Reset_Angles = false;

		if (player.team_id != Drv.read<char>(player.enemy_state + 0xf60))
		{
			if (isVisible(player.mesh))
			{

				Vector3 PlayerBone = BoneArray(player.mesh, 66);
				Vector3 BonePosition = Vector3(PlayerBone.x, PlayerBone.y, PlayerBone.z);
				Vector3 PlayerPosition = ProjectWorldToScreen(BonePosition);

				if (PlayerPosition.x != 0 || PlayerPosition.y != 0 || PlayerPosition.z != 0)
				{

					if (GetDistance(PlayerPosition.x, PlayerPosition.y, PlayerPosition.z, center_x, center_y))
					{

						if (Copy_lines)
							ImGui::GetOverlayDrawList()->AddLine(ImVec2(center_x, center_y), ImVec2(PlayerPosition.x, PlayerPosition.y), ImColor(255, 0, 0, 255), 1.0f);

						if (GetAsyncKeyState(VK_RBUTTON) && Copy_Aimbot)
						{
							aimbot(PlayerPosition.x, PlayerPosition.y);

							if (freezepawns)
								Drv.write<int>(player.pawn + 0x99C, 0.0072f);
						}

						if (Copy_Triggerbot)
						{
							mouse_event(MOUSEEVENTF_LEFTDOWN, DWORD(NULL), DWORD(NULL), DWORD(0x0002), ULONG_PTR(NULL));
							Sleep(delay); // Milliseconds
							mouse_event(MOUSEEVENTF_LEFTUP, DWORD(NULL), DWORD(NULL), DWORD(0x0004), ULONG_PTR(NULL));

						}

					}

				}
			}
		}

		if (!freezepawns)
			Drv.write<int>(player.pawn + 0x99C, 1);
	}
}

void excucute_exploits()
{
	// Depreciated 0x3A0
}

void WorldCache()
{
	while (true)
	{
		game::globals::GWorld = Drv.read<uintptr_t>(cheat::ModuleBase + 0x4C6D920);
		game::globals::Gameinstance = Drv.read<uintptr_t>(game::globals::GWorld + 0x190);
		game::globals::Localplayer = Drv.read<uintptr_t>(Drv.read<uintptr_t>(game::globals::Gameinstance + 0x38));
		game::globals::PlayerController = Drv.read<uintptr_t>(game::globals::Localplayer + 0x30);
		game::globals::LocalPawn = Drv.read<uintptr_t>(game::globals::PlayerController + 0x388);
		game::globals::PlayerState = Drv.read<uintptr_t>(game::globals::LocalPawn + 0x338);
		game::globals::RootComponent = Drv.read<uintptr_t>(game::globals::LocalPawn + 0x158);
		game::globals::relativelocation = Drv.read<Vector3>(game::globals::RootComponent + 0x15c);
		game::globals::Persistentlevel = Drv.read<uintptr_t>(game::globals::GWorld + 0x30);
		game::globals::ActorCount = Drv.read<DWORD>(game::globals::Persistentlevel + 0xA8);
		game::globals::AActors = Drv.read<uintptr_t>(game::globals::Persistentlevel + 0xA0);
		game::globals::UworldPointer = (uintptr_t)game::globals::GWorld;
		GetCamera(game::globals::RootComponent);

		//excucute_exploits();

		if (game::globals::GWorld != game::globals::UworldPointer) {
			if (!playerlist.empty())
				playerlist.clear();
			game::globals::UworldPointer = game::globals::GWorld;
		}

		Sleep(1750);
	}
}

void ScanActors()
{
	while (true)
	{
		if (fastbullets)
		{
			uintptr_t CurrentWeapon = Drv.read<uintptr_t>(game::globals::LocalPawn + 0x7D0);
			uintptr_t WeaponData = Drv.read<uintptr_t>(CurrentWeapon + 0x368);
			Drv.write<float>(game::globals::PlayerController + 0xC0, -1);
			Drv.write<float>(CurrentWeapon + 0xC0, 9999);
			Drv.write<uint8_t>(WeaponData + 0x680, 1);
			Drv.write<int>(CurrentWeapon + 0x99C, 10);
			Drv.write<int>(CurrentWeapon + 0x99C + 0x8, 10);
		}

		for (int i = 0; i < game::globals::ActorCount; ++i)
		{
			uintptr_t actor = Drv.read<DWORD_PTR>(game::globals::AActors + i * 0x8);
			uintptr_t mesh = Drv.read<uintptr_t>(actor + 0x378);
			float player_check = Drv.read<float>(actor + 0x1A80);
			ImU32 color;

			if (brainbow)
				color = rainbow();
			else if (isVisible(mesh))
				color = ImColor(255, 255, 255, 255);
			else
				color = ImColor(255, 0, 0, 255);

			if (player_check == 10)
			{
				uintptr_t EnemyState = Drv.read<uintptr_t>(actor + 0x338);
				INT32 team_id = Drv.read<INT32>(game::globals::PlayerState + 0xf60);

				game::actor player
				{
					actor,
					mesh,
					EnemyState,
					team_id,
					player_check,
					true,
					color
				};

				if (!playerlist.empty()) {
					auto found_player = std::find(playerlist.begin(), playerlist.end(), player);
					if (found_player == playerlist.end())
						playerlist.push_back(player);
				}
				else
					playerlist.push_back(player);

			}
		}

	}
}

void render()
{
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (GetAsyncKeyState(VK_INSERT))
	{
		DrawMenu = !DrawMenu;
		Sleep(200);
	}

	if (DrawMenu)
	{
		ImGui::SetNextWindowSize({ 850, 600 });
		ImGui::Begin("(Insert goofy p2c name) by LeProxy", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImDrawList* DrawList{ };
		ImGuiTreeNodeFlags Header_Flags = ImGuiTreeNodeFlags_NoTreePushOnOpen;
		ImGui::Spacing();

		if (ImGui::CollapsingHeader("Aimbot", Header_Flags)) {
			ImGui::Checkbox("Mouse Aimbot", &Aimbot);
			ImGui::Checkbox("lines", &TargetLines);
			ImGui::Checkbox("Nigger", &freezepawns);
			ImGui::SliderInt("Speed", &smooth, 1.0f, 20.0f);
			ImGui::SliderInt("Humanise Aim", &offset_value, 0, 10);
			ImGui::SliderFloat("FieldOfView", &AimFOV, 0.f, 500.f);
			ImGui::Spacing();
		}
		if (ImGui::CollapsingHeader("Triggerbot", Header_Flags)) {
			ImGui::Checkbox("Trigger", &autofire);
			ImGui::SliderInt("Delay MilliSeconds", &delay, 1, 10);
			ImGui::Spacing();
		}
		if (ImGui::CollapsingHeader("Weapon", Header_Flags)) {
			static int WeaponTabs;

			if (ImGui::Button("Assualt Rifle")) WeaponTabs = MENU_TAB_RIFLE;
			ImGui::SameLine();
			if (ImGui::Button("Shot Gun")) WeaponTabs = MENU_TAB_SHOTGUN;
			ImGui::SameLine();
			if (ImGui::Button("SMG")) WeaponTabs = MENU_TAB_SMG;
			ImGui::SameLine();
			if (ImGui::Button("Pistol")) WeaponTabs = MENU_TAB_PISTOL;
			ImGui::SameLine();
			if (ImGui::Button("Sniper")) WeaponTabs = MENU_TAB_SNIPER;

			ImGui::Spacing();
			ImGui::Checkbox("Weapon Configs", &WeaponConfigs);
			ImGui::Spacing();

			if (!WeaponConfigs) WeaponTabs = MENU_TAB_DEFAULT;

			else if (WeaponTabs == MENU_TAB_RIFLE && WeaponConfigs)
			{
				ImGui::TextColored(ImColor(255, 0, 0, 255), "Assualt Rifle");
				ImGui::SliderFloat("Smoothness", &smoothness1, 1.0f, 10.f);
				ImGui::SliderFloat("FieldOfView", &fov1, 25.f, 800.f);
			}
			else if (WeaponTabs == MENU_TAB_SHOTGUN && WeaponConfigs)
			{
				ImGui::TextColored(ImColor(255, 0, 0, 255), "Shotgun");
				ImGui::SliderFloat("Smoothness", &smoothness2, 1.0f, 10.f);
				ImGui::SliderFloat("FieldOfView", &fov2, 25.f, 800.f);
			}
			else if (WeaponTabs == MENU_TAB_SMG && WeaponConfigs)
			{
				ImGui::TextColored(ImColor(255, 0, 0, 255), "SubMachine Gun");
				ImGui::SliderFloat("Smoothness", &smoothness3, 1.0f, 10.f);
				ImGui::SliderFloat("FieldOfView", &fov3, 25.f, 800.f);
			}
			else if (WeaponTabs == MENU_TAB_PISTOL && WeaponConfigs)
			{
				ImGui::TextColored(ImColor(255, 0, 0, 255), "Pistol");
				ImGui::SliderFloat("Smoothness", &smoothness4, 1.0f, 10.f);
				ImGui::SliderFloat("FieldOfView", &fov4, 25.f, 800.f);
			}
			else if (WeaponTabs == MENU_TAB_SNIPER && WeaponConfigs)
			{
				ImGui::TextColored(ImColor(255, 0, 0, 255), "Sniper");
				ImGui::SliderFloat("Smoothness", &smoothness5, 1.0f, 10.f);
				ImGui::SliderFloat("FieldOfView", &fov5, 25.f, 800.f);
			}

			ImGui::Spacing();

		}
		if (ImGui::CollapsingHeader("Player", Header_Flags)) {
			//ImGui::Text("this is a exmaple of a collapsable header named Player");
			ImGui::Combo("box", &box_type, box, sizeof(box) / sizeof(*box));
			ImGui::Checkbox("fill", &fill); 
			ImGui::Checkbox("distance", &distance);
			ImGui::Checkbox("lines", &Snaplines);
			ImGui::Checkbox("Skeleton", &skell);
			ImGui::Checkbox("head", &bhead);
			ImGui::Checkbox("Rainbow", &brainbow);
			ImGui::Spacing();
		}
		if (ImGui::CollapsingHeader("Radar", Header_Flags)) {
			ImGui::Text("Radar :)");
			ImGui::Spacing();
		}
		if (ImGui::CollapsingHeader("Exploits", Header_Flags)) {
			ImGui::Checkbox("Fast Bullets", &fastbullets);
			ImGui::Checkbox("Magic Bullets", &MaGicBullEts);
			ImGui::SliderFloat("FieldOfView", &fieldofview, 0.f, 1000.f);
			ImGui::Spacing();
		}

		ImGui::End();
	}

	blackballs();
	ImGui::EndFrame();
	D3dDevice->SetRenderState(D3DRS_ZENABLE, false);
	D3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	D3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
	D3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	if (D3dDevice->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		D3dDevice->EndScene();
	}
	HRESULT result = D3dDevice->Present(NULL, NULL, NULL, NULL);

	if (result == D3DERR_DEVICELOST && D3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
	{
		ImGui_ImplDX9_InvalidateDeviceObjects();
		D3dDevice->Reset(&d3dpp);
		ImGui_ImplDX9_CreateDeviceObjects();
	}
}

void xShutdown()
{
	TriBuf->Release();
	D3dDevice->Release();
	p_Object->Release();

	DestroyWindow(Window);
	UnregisterClass((L"Sony Overlay"), NULL);
}

void xMainLoop()
{
	static RECT old_rc;
	ZeroMemory(&Message, sizeof(MSG));

	while (Message.message != WM_QUIT)
	{

		if (PeekMessage(&Message, Window, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}

		HWND hwnd_active = GetForegroundWindow();

		if (hwnd_active == cheat::hWnd) {
			HWND hwndtest = GetWindow(hwnd_active, GW_HWNDPREV);
			SetWindowPos(Window, hwndtest, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}

		RECT rc;
		POINT xy;

		ZeroMemory(&rc, sizeof(RECT));
		ZeroMemory(&xy, sizeof(POINT));
		GetClientRect(cheat::hWnd, &rc);
		ClientToScreen(cheat::hWnd, &xy);
		rc.left = xy.x;
		rc.top = xy.y;

		ImGuiIO& io = ImGui::GetIO();
		io.ImeWindowHandle = cheat::hWnd;
		io.DeltaTime = 1.0f / 60.f;

		POINT p;
		GetCursorPos(&p);
		io.MousePos.x = p.x - xy.x;
		io.MousePos.y = p.y - xy.y;

		if (GetAsyncKeyState(VK_LBUTTON)) {
			io.MouseDown[0] = true;
			io.MouseClicked[0] = true;
			io.MouseClickedPos[0].x = io.MousePos.x;
			io.MouseClickedPos[0].x = io.MousePos.y;
		}
		else
			io.MouseDown[0] = false;

		if (rc.left != old_rc.left || rc.right != old_rc.right || rc.top != old_rc.top || rc.bottom != old_rc.bottom)
		{
			old_rc = rc;

			Width = rc.right;
			Height = rc.bottom;

			d3dpp.BackBufferWidth = Width;
			d3dpp.BackBufferHeight = Height;
			SetWindowPos(Window, (HWND)0, xy.x, xy.y, Width, Height, SWP_NOREDRAW);
			D3dDevice->Reset(&d3dpp);
		}

		center_x = Width / 2;
		center_y = Height / 2;

		render();

	}

	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	DestroyWindow(Window);
}

LRESULT CALLBACK WinProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, Message, wParam, lParam))
		return true;

	switch (Message)
	{
	case WM_DESTROY:
		xShutdown();
		PostQuitMessage(0);
		exit(4);
		break;
	case WM_SIZE:
		if (D3dDevice != NULL && wParam != SIZE_MINIMIZED)
		{
			ImGui_ImplDX9_InvalidateDeviceObjects();
			d3dpp.BackBufferWidth = LOWORD(lParam);
			d3dpp.BackBufferHeight = HIWORD(lParam);
			HRESULT hr = D3dDevice->Reset(&d3dpp);
			if (hr == D3DERR_INVALIDCALL)
				IM_ASSERT(0);
			ImGui_ImplDX9_CreateDeviceObjects();
		}
		break;
	default:
		return DefWindowProc(hWnd, Message, wParam, lParam);
		break;
	}
	return 0;
}

void setup_window()
{
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)SetWindowToTarget, 0, 0, 0);

	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(wc);
	wc.lpszClassName = (L"Sony Overlay");
	wc.lpfnWndProc = WinProc;
	RegisterClassEx(&wc);

	GetClientRect(cheat::hWnd, &GameRect);
	POINT xy;
	ClientToScreen(cheat::hWnd, &xy);
	GameRect.left = xy.x;
	GameRect.top = xy.y;

	Width = GameRect.right;
	Height = GameRect.bottom;

	Window = CreateWindowEx(NULL, (L"Sony Overlay"), (L"Sony Overlay Util"), WS_POPUP | WS_VISIBLE, 0, 0, Width, Height, 0, 0, 0, 0);

	DwmExtendFrameIntoClientArea(Window, &Margin);
	SetWindowLong(Window, GWL_EXSTYLE, WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_LAYERED);
	ShowWindow(Window, SW_SHOW);
	UpdateWindow(Window);
}

void start_cheat()
{
	SonyDriverHelper::api::Init();

	setup_window();
	xInitD3d();

	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)WorldCache, NULL, NULL, NULL); Sleep(200);
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ScanActors, NULL, NULL, NULL); Sleep(200);

	xMainLoop();
	xShutdown();
}

int main()
{
	SetConsoleTitleA("LeProxy");

	std::cout << "\n Please load the driver found in the same folder before hooking to the game" << std:: endl;
	Sleep(3000);
	std::cout << " Press keybind F1 to hook to fortnite when in lobby" << std::endl;

	while (!cheat::ModuleBase)
	{
		if (GetAsyncKeyState(VK_F1))
		{
			cheat::hWnd = FindWindowA("UnrealWindow", "Fortnite  ");
			Sleep(200);

			if (!cheat::hWnd)
			{
				std::cout << " Failed to find fortnite" << std::endl;
				Sleep(2000);
				exit(0);
			}

			std::cout << " Found fortnite" << std::endl;
			ProcessId = GetPid(L"FortniteClient-Win64-Shipping.exe");
			cheat::ModuleBase = Drv.GetModuleBase("FortniteClient-Win64-Shipping.exe");
		}
	}

	if (!ProcessId) {
		std::cout << " Driver Error: Failed to get games PID please restart and remap driver" << std::endl;
		Sleep(2000);
		return 1;
	}

	if (!cheat::ModuleBase) {
		std::cout << " Driver Error: Failed to get games base address please restart and remap driver" << std::endl;
		Sleep(2000);
		return 1;
	}

	MessageBoxA(cheat::hWnd, "Attached to fortnite battle royale", "hooked!", 0);
	Sleep(1000);
	start_cheat();

	exit(0);
}
