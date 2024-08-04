#include "ImGui/imgui.h"
#include "drawing.h"

void AddLine(FVector2D start, FVector2D end, ImU32 color, float thickness)
{
	ImDrawList* drawList = ImGui::GetOverlayDrawList();
	drawList->AddLine(ImVec2(start.x, start.y), ImVec2(end.x, end.y), color, thickness);
}

void FullBox(int X, int Y, int W, int H, const ImColor color, int thickness)
{
	AddLine(FVector2D{ (float)X, (float)Y }, FVector2D{ (float)(X + W), (float)Y }, color, thickness);
	AddLine(FVector2D{ (float)(X + W), (float)Y }, FVector2D{ (float)(X + W), (float)(Y + H) }, color, thickness);
	AddLine(FVector2D{ (float)X, (float)(Y + H) }, FVector2D{ (float)(X + W), (float)(Y + H) }, color, thickness);
	AddLine(FVector2D{ (float)X, (float)Y }, FVector2D{ (float)X, (float)(Y + H) }, color, thickness);
}