#pragma once
#include "sdk.h"
#include <thread>
#include <unordered_map>
#include "D3DX/d3dx9math.h"
#include <string>

#define PI 3.14159265358979323846f

int screenWidth = GetSystemMetrics(SM_CXSCREEN);
int screenHeight = GetSystemMetrics(SM_CYSCREEN);

namespace Features
{
	inline bool LevelActorCaching;
	inline bool rRenderCount;
	inline bool rFullbox;
	inline bool rRank;
	inline bool rSkeleton;
	inline bool rWeapon;
	inline int rFovSize = 200;
	inline bool rAimbot;
	inline int rSmooth = 5;
	inline int rAimkey = VK_RBUTTON;
	inline bool rFovCircle;
	inline bool rDistance;
}

namespace Offsets {
	uint64_t
		UWorld = 0x118011A8,
		GNames = 0x12BA9140,
		GameState = 0x160,
		PlayerArray = 0x2A8,
		GameInstance = 0x1D8,
		LocalPlayers = 0x38,
		PlayerController = 0x30,
		LocalPawn = 0x338,
		PlayerState = 0x2B0,
		RootComponent = 0x198,
		PersistentLevel = 0x30,
		AActors = 0xA0,
		ActorCount = 0xA8,
		ReviveFromDBNOTime = 0x4C30,
		Mesh = 0x318,
		TeamIndex = 0x1211,
		Platform = 0x438,
		PawnPrivate = 0x308,
		RelativeLocation = 0x120,
		PrimaryPickupItemEntry = 0x350,
		ItemDefinition = 0x18,
		Rarity = 0x9A,
		ItemName = 0x40,
		Levels = 0x178,
		WeaponData = 0x510,
		AmmoCount = 0xEEC,
		bIsTargeting = 0x581,
		ComponentVelocity = 0x168,
		TargetedFortPawn = 0x18C8,
		CurrentWeapon = 0xA68,
		BoneArray = 0x5B8,
		BoneCache = 0x600,
		LastSubmitTime = 0x2E8,
		LastRenderTime = 0x2F0,
		ComponentToWorld = 0x1C0;
}
#define FortPTR uintptr_t

#define CURRENT_CLASS reinterpret_cast<uintptr_t>(this)
#define DECLARE_MEMBER(type, name, offset) type name() { return read<type>(CURRENT_CLASS + offset); }
#define DECLARE_MEMBER_DIRECT(type, name, base, offset) type name() { read<type>(base + offset); }
#define DECLARE_MEMBER_BITFIELD(type, name, offset, index) type name() { read<type>(CURRENT_CLASS + offset) & (1 << index); }
#define APPLY_MEMBER(type, name, offset) void name( type val ) { write<type>(CURRENT_CLASS + offset, val); }
#define APPLY_MEMBER_BITFIELD(type, name, offset, index) void name( type val ) { write(CURRENT_CLASS + offset, |= val << index); }

DWORD_PTR Uworld_Cam;

class UObject {
public:
	FortPTR GetAddress()
	{
		return (FortPTR)this;
	}

	DECLARE_MEMBER(int, GetObjectID, 0x18);

	__forceinline operator uintptr_t() { return (FortPTR)this; }
};

class USceneComponent : public UObject
{
public:
	DECLARE_MEMBER(Vector3, RelativeLocation, Offsets::RelativeLocation);
	DECLARE_MEMBER(FBoxSphereBounds, Bounds, Offsets::RelativeLocation - 0x38);
	DECLARE_MEMBER(Vector3, GetComponentVelocity, Offsets::ComponentVelocity);
	APPLY_MEMBER(Vector3, K2_SetActorLocation, Offsets::RelativeLocation);
};

class AActor : public UObject {
public:
	DECLARE_MEMBER(USceneComponent*, RootComponent, Offsets::RootComponent);

	Vector3 K2_GetActorLocation_AlwaysCached() {
		static std::unordered_map<int, Vector3> CachedLocations;
		auto it = CachedLocations.find((int)this->GetAddress());
		if (it != CachedLocations.end()) {
			return it->second;
		}

		Vector3 ReturnValue = this->RootComponent()->RelativeLocation();

		CachedLocations[(int)this->GetAddress()] = ReturnValue;

		return ReturnValue;
	}

	Vector3 K2_GetActorLocation_Cached() {
		static std::unordered_map<int, Vector3> CachedLocations;
		if (Features::LevelActorCaching)
		{
			auto it = CachedLocations.find((int)this->GetAddress());
			if (it != CachedLocations.end()) {
				return it->second;
			}
		}

		Vector3 ReturnValue = this->RootComponent()->RelativeLocation();

		CachedLocations[(int)this->GetAddress()] = ReturnValue;

		return ReturnValue;
	}
};

class USkeletalMeshComponent : public AActor {
public:
	__forceinline Vector3 GetSocketLocation(int bone_id)
	{
		int IsCached = read<int>((FortPTR)this + 0x600);
		auto BoneTransform = read<FTransform>(read<uintptr_t>((FortPTR)this + 0x10 * IsCached + 0x5B8) + 0x60 * bone_id);

		FTransform ComponentToWorld = read<FTransform>((FortPTR)this + 0x1c0);

		D3DMATRIX Matrix;
		Matrix = MatrixMultiplication(BoneTransform.ToMatrixWithScale(), ComponentToWorld.ToMatrixWithScale());
		return Vector3(Matrix._41, Matrix._42, Matrix._43);
	}

	__forceinline bool WasRecentlyRendered(float tolerence)
	{
		float LastSubmitTime = read<float>(this->GetAddress() + Offsets::LastSubmitTime);
		float LastRenderTimeOnScreen = read<float>(this->GetAddress() + Offsets::LastRenderTime);

		return LastRenderTimeOnScreen + tolerence >= LastSubmitTime;
	}

	auto is_shootable(Vector3 lur, Vector3 bone) -> bool {

			if (lur.x >= bone.x - 20 && lur.x <= bone.x + 20 && lur.y >= bone.y - 20 && lur.y <= bone.y + 20 && lur.z >= bone.z - 30 && lur.z <= bone.z + 30) return true;
			else return false;

	}
};

struct CameraInfo
{
	Vector3 location;
	Vector3 rotation;
	float fov;
};

CameraInfo Copy_CameraInfo;

FortPTR Copy_PlayerController_Camera;

CameraInfo camera;

CameraInfo GetCameraInfo()
{
		if (Copy_PlayerController_Camera)
		{
			auto location_pointer = read<uintptr_t>(Uworld_Cam + 0x110); //110
			auto rotation_pointer = read<uintptr_t>(Uworld_Cam + 0x120); //120

			struct RotationInfo
			{
				double pitch;
				char pad_0008[24];
				double yaw;
				char pad_0028[424];
				double roll;
			};
			RotationInfo RotInfo;

			RotInfo = read<RotationInfo>(rotation_pointer);

			camera.location = read<Vector3>(location_pointer);
			camera.rotation.x = asin(RotInfo.roll) * (180.0 / PI);
			camera.rotation.y = ((atan2(RotInfo.pitch * -1, RotInfo.yaw) * (180.0 / PI)) * -1) * -1;
			camera.fov = read<float>(Copy_PlayerController_Camera + 0x394) * 90.f;

			return { camera.location, camera.rotation, camera.fov };
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

class ULevel : public UObject
{
public:
	DECLARE_MEMBER(DWORD, ActorCount, Offsets::ActorCount);
	DECLARE_MEMBER(FortPTR, AActors, Offsets::AActors);
};

class AFortPlayerState : public AActor
{
public:
	DECLARE_MEMBER(int, TeamIndex, Offsets::TeamIndex);
	DECLARE_MEMBER(AActor*, PawnPrivate, Offsets::PawnPrivate);
	DECLARE_MEMBER(DWORD_PTR, Platform, Offsets::Platform);
};

class UFortItemDefinition : public AActor
{
public:
	DECLARE_MEMBER(EFortRarity, Rarity, Offsets::Rarity);
	DECLARE_MEMBER(int, RarityInt, Offsets::Rarity);
	std::string ItemName()
	{
		std::string PlayersWeaponName = "";
		uint64_t ItemName = read<uint64_t>((FortPTR)this + Offsets::ItemName);
		if (!ItemName) return "";

		uint64_t FData = read<uint64_t>(ItemName + 0x28);
		int FLength = read<int>(ItemName + 0x30);

		if (FLength > 0 && FLength < 50) {

			wchar_t* WeaponBuffer = new wchar_t[FLength];
			mem::read_physical((PVOID)FData, (PVOID)WeaponBuffer, FLength * sizeof(wchar_t));
			std::wstring wstr_buf(WeaponBuffer);
			PlayersWeaponName.append(std::string(wstr_buf.begin(), wstr_buf.end()));

			delete[] WeaponBuffer;
		}
		return PlayersWeaponName;
	}
};

class AFortWeapon : public AActor
{
public:
	std::string GetWeaponName(uintptr_t Player) {
		std::string PlayersWeaponName = "";
		uint64_t CurrentWeapon = read<uint64_t>((FortPTR)Player + Offsets::CurrentWeapon);
		uint64_t weapondata = read<uint64_t>(CurrentWeapon + Offsets::WeaponData);
		uint64_t AmmoCount = read<uint64_t>(CurrentWeapon + Offsets::AmmoCount);
		uint64_t ItemName = read<uint64_t>(weapondata + Offsets::ItemName);
		if (!ItemName) return "";

		uint64_t FData = read<uint64_t>(ItemName + 0x28);
		int FLength = read<int>(ItemName + 0x30);

		if (FLength > 0 && FLength < 50) {

			wchar_t* WeaponBuffer = new wchar_t[FLength];
			mem::read_physical((void*)FData, (PVOID)WeaponBuffer, FLength * sizeof(wchar_t));
			std::wstring wstr_buf(WeaponBuffer);
			if (AmmoCount != 0) PlayersWeaponName.append(std::string(wstr_buf.begin(), wstr_buf.end()) + "[" + std::to_string(AmmoCount) + "]");
			else PlayersWeaponName.append(std::string(wstr_buf.begin(), wstr_buf.end()));

			delete[] WeaponBuffer;
		}
		return PlayersWeaponName;
	}

	float GetProjectileSpeed()
	{
		return read<float>(CURRENT_CLASS + 0x1CE0);
	}

	float GetGravityScale()
	{
		return read<float>(CURRENT_CLASS + 0x1CE4);
	}

	DECLARE_MEMBER(UFortItemDefinition*, WeaponData, Offsets::WeaponData);
	DECLARE_MEMBER(int, AmmoCount, Offsets::AmmoCount);
	DECLARE_MEMBER(bool, IsTargeting, Offsets::bIsTargeting);
};

class AFortPlayerPawn : public AActor
{
public:
	DECLARE_MEMBER(float, ReviveFromDBNOTime, Offsets::ReviveFromDBNOTime);
	DECLARE_MEMBER(AFortPlayerState*, PlayerState, Offsets::PlayerState);
	DECLARE_MEMBER(USkeletalMeshComponent*, Mesh, Offsets::Mesh);
	DECLARE_MEMBER(AFortWeapon*, CurrentWeapon, Offsets::CurrentWeapon);
};

class APlayerController : public AActor
{
public:
	D3DMATRIX Matrix(Vector3 rot, Vector3 origin = Vector3(0.f,0.f,0.f)) {
		float radPitch = (rot.x * float(PI) / 180.f);
		float radYaw = (rot.y * float(PI) / 180.f);
		float radRoll = (rot.z * float(PI) / 180.f);

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

	Vector2 ProjectWorldLocationToScreen(Vector3 WorldLocation)
	{
		CameraInfo viewInfo = GetCameraInfo();
		D3DMATRIX tempMatrix = Matrix(viewInfo.rotation);
		Vector3 vAxisX = Vector3(tempMatrix.m[0][0], tempMatrix.m[0][1], tempMatrix.m[0][2]);
		Vector3 vAxisY = Vector3(tempMatrix.m[1][0], tempMatrix.m[1][1], tempMatrix.m[1][2]);
		Vector3 vAxisZ = Vector3(tempMatrix.m[2][0], tempMatrix.m[2][1], tempMatrix.m[2][2]);

		Vector3 vDelta = WorldLocation - viewInfo.location;
		Vector3 vTransformed = Vector3(vDelta.Dot(vAxisY), vDelta.Dot(vAxisZ), vDelta.Dot(vAxisX));

		if (vTransformed.z < 1.f)
			vTransformed.z = 1.f;

		return Vector2((screenWidth / 2.0f) + vTransformed.x * (((screenWidth / 2.0f) / tanf(viewInfo.fov * (float)PI / 360.f))) / vTransformed.z, (screenHeight / 2.0f) - vTransformed.y * (((screenWidth / 2.0f) / tanf(viewInfo.fov * (float)PI / 360.f))) / vTransformed.z);
	}

	DECLARE_MEMBER(FortPTR, AcknowledgedPawn, Offsets::LocalPawn);
	DECLARE_MEMBER(AFortPlayerPawn*, TargetedFortPawn, Offsets::TargetedFortPawn);
};

class ULocalPlayer : public AFortPlayerPawn
{
public:
	DECLARE_MEMBER(APlayerController*, PlayerController, Offsets::PlayerController);
};

class UGameInstance : public AActor
{
public:
	DECLARE_MEMBER(DWORD_PTR, LocalPlayers, Offsets::LocalPlayers);
};

class AGameStateBase : public UObject
{
public:
	DECLARE_MEMBER(TArray, PlayerArray, Offsets::PlayerArray);
};

class UWorld : public AActor
{
public:
	DECLARE_MEMBER(UGameInstance*, OwningGameInstance, Offsets::GameInstance);
	DECLARE_MEMBER(ULevel*, PersistentLevel, Offsets::PersistentLevel);
	DECLARE_MEMBER(AGameStateBase*, GameState, Offsets::GameState);
};

class FFortItemEntry : public AActor
{
public:
};

class AFortPickup : public AActor
{
public:
	DECLARE_MEMBER(UFortItemDefinition*, ItemDefinition, Offsets::PrimaryPickupItemEntry + Offsets::ItemDefinition);
};

namespace Cached
{
	APlayerController* PlayerController;
	AFortPlayerState* PlayerState;
	AFortPlayerState* LocalPlayerState;
	AFortPlayerPawn* LocalPawn;
	USceneComponent* LocalRootComponent;
	ULocalPlayer* LocalPlayer;
	ULevel* PersistentLevel;
	UWorld* World;
	AGameStateBase* GameState;
	AFortWeapon* CurrentWeapon;


	inline AFortPlayerPawn* TargetEntity = 0;
	inline float ClosestDistance = FLT_MAX;


	Vector3 localactorpos;
}

double GetCrossDistance(double x1, double y1, double x2, double y2) {
	return sqrt(pow((x2 - x1), 2) + pow((y2 - y1), 2));
}