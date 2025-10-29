#pragma once

#include "Module.h"
#include "Vector2D.h"
#include "SDL3/SDL.h"

class Render : public Module
{
public:

	Render();

	// Destructor
	virtual ~Render();

	// Called before render is available
	bool Awake();

	// Called before the first frame
	bool Start();

	// Called each loop iteration
	bool PreUpdate();
	bool Update(float dt);
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	void SetViewPort(const SDL_Rect& rect);
	void ResetViewPort();

	// Drawing
	bool DrawTexture(SDL_Texture* texture, int x, int y, const SDL_Rect* section = NULL, float speed = 1.0f, double angle = 0, int pivotX = INT_MAX, int pivotY = INT_MAX) const;
	bool DrawRectangle(const SDL_Rect& rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255, bool filled = true, bool useCamera = true) const;
	bool DrawLine(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255, bool useCamera = true) const;
	bool DrawCircle(int x1, int y1, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255, bool useCamera = true) const;

	// Set background color
	void SetBackgroundColor(SDL_Color color);

	// Draw text (used for debug help)
	void DrawText(const char* text, int x, int y, int scale = 2, Uint8 r = 255, Uint8 g = 255, Uint8 b = 255, Uint8 a = 255, bool useCamera = false) const;

private:
	void DrawGlyph(char c, int x, int y, int scale, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool useCamera) const;

public:
	SDL_Renderer* renderer = nullptr;
	SDL_Rect camera = { 0,0,0,0 };
	SDL_Rect viewport = { 0,0,0,0 };
	SDL_Color background = { 0,0,0,0 };

private:
	bool vsync = false;
};
