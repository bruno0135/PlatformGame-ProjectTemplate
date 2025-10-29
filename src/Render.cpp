#include "Engine.h"
#include "Window.h"
#include "Render.h"
#include "Log.h"
#include <cmath>
#include <cctype>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Render::Render() : Module()
{
	name = "render";
	background.r = 0;
	background.g = 0;
	background.b = 0;
	background.a = 0;
	renderer = nullptr;
	camera = { 0,0,0,0 };
	viewport = { 0,0,0,0 };
}

// Destructor
Render::~Render()
{
}

// Called before render is available
bool Render::Awake()
{
	LOG("Create SDL rendering context");
	bool ret = true;

	int scale = Engine::GetInstance().window->GetScale();
	SDL_Window* window = Engine::GetInstance().window->window;

	//L05 TODO 5 - Load the configuration of the Render module

	renderer = SDL_CreateRenderer(window, nullptr);

	if (renderer == NULL)
	{
		LOG("Could not create the renderer! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	else
	{
		if (configParameters.child("vsync").attribute("value").as_bool())
		{
			if (!SDL_SetRenderVSync(renderer, 1))
			{
				LOG("Warning: could not enable vsync: %s", SDL_GetError());
			}
			else
			{
				LOG("Using vsync");
			}
		}

		camera.w = Engine::GetInstance().window->width * scale;
		camera.h = Engine::GetInstance().window->height * scale;
		camera.x = 0;
		camera.y = 0;
	}

	return ret;
}

// Called before the first frame
bool Render::Start()
{
	LOG("render start");
	if (!SDL_GetRenderViewport(renderer, &viewport))
	{
		LOG("SDL_GetRenderViewport failed: %s", SDL_GetError());
	}
	return true;
}

// Called each loop iteration
bool Render::PreUpdate()
{
	SDL_RenderClear(renderer);
	return true;
}

bool Render::Update(float dt)
{
	return true;
}

bool Render::PostUpdate()
{
	if (Engine::GetInstance().IsHelpShown())
	{
		SDL_Rect panel = { 16, 16, camera.w - 32, 240 };
		DrawRectangle(panel, 0, 0, 0, 180, true, false);
		DrawRectangle(panel, 255, 255, 255, 220, false, false);

		int x = 32;
		int y = 36;

		DrawText("CONTROLS:", x, y, 3); y += 28;
		DrawText("- W / A / S / D  ->  Move player", x, y); y += 18;
		DrawText("- SPACE          ->  Jump", x, y); y += 18;
		DrawText("- T              ->  Teleport (test)", x, y); y += 18;
		DrawText("- ESC            ->  Exit game", x, y); y += 28;

		DrawText("DEBUG MODE:", x, y, 3); y += 28;
		DrawText("- H              ->  Show / Hide help", x, y); y += 18;
		DrawText("- F9             ->  Show collisions and logic (debug draw)", x, y); y += 18;
		DrawText("- F10            ->  Toggle God Mode (fly and invincible)", x, y); y += 18;
		DrawText("- F11            ->  Toggle FPS cap between 30 / 60", x, y);
	}

	SDL_SetRenderDrawColor(renderer, background.r, background.g, background.g, background.a);
	SDL_RenderPresent(renderer);
	return true;
}

// Called before quitting
bool Render::CleanUp()
{
	LOG("Destroying SDL render");
	SDL_DestroyRenderer(renderer);
	return true;
}

void Render::SetBackgroundColor(SDL_Color color)
{
	background = color;
}

void Render::SetViewPort(const SDL_Rect& rect)
{
	SDL_SetRenderViewport(renderer, &rect);
}

void Render::ResetViewPort()
{
	SDL_SetRenderViewport(renderer, &viewport);
}

// Blit to screen
bool Render::DrawTexture(SDL_Texture* texture, int x, int y, const SDL_Rect* section, float speed, double angle, int pivotX, int pivotY) const
{
	bool ret = true;
	int scale = Engine::GetInstance().window->GetScale();

	SDL_FRect rect;
	rect.x = static_cast<float>((int)(camera.x * speed) + x * scale);
	rect.y = static_cast<float>((int)(camera.y * speed) + y * scale);

	if (section != NULL)
	{
		rect.w = static_cast<float>(section->w * scale);
		rect.h = static_cast<float>(section->h * scale);
	}
	else
	{
		float tw = 0.0f, th = 0.0f;
		if (!SDL_GetTextureSize(texture, &tw, &th))
		{
			LOG("SDL_GetTextureSize failed: %s", SDL_GetError());
			return false;
		}
		rect.w = tw * scale;
		rect.h = th * scale;
	}

	const SDL_FRect* src = NULL;
	SDL_FRect srcRect;
	if (section != NULL)
	{
		srcRect.x = static_cast<float>(section->x);
		srcRect.y = static_cast<float>(section->y);
		srcRect.w = static_cast<float>(section->w);
		srcRect.h = static_cast<float>(section->h);
		src = &srcRect;
	}

	SDL_FPoint* p = NULL;
	SDL_FPoint pivot;
	if (pivotX != INT_MAX && pivotY != INT_MAX)
	{
		pivot.x = static_cast<float>(pivotX);
		pivot.y = static_cast<float>(pivotY);
		p = &pivot;
	}

	int rc = SDL_RenderTextureRotated(renderer, texture, src, &rect, angle, p, SDL_FLIP_NONE) ? 0 : -1;
	if (rc != 0)
	{
		LOG("Cannot blit to screen. SDL_RenderTextureRotated error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool Render::DrawRectangle(const SDL_Rect& rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool filled, bool use_camera) const
{
	bool ret = true;
	int scale = Engine::GetInstance().window->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	SDL_FRect rec;
	if (use_camera)
	{
		rec.x = static_cast<float>(camera.x + rect.x * scale);
		rec.y = static_cast<float>(camera.y + rect.y * scale);
	}
	else
	{
		rec.x = static_cast<float>(rect.x * scale);
		rec.y = static_cast<float>(rect.y * scale);
	}
	rec.w = static_cast<float>(rect.w * scale);
	rec.h = static_cast<float>(rect.h * scale);

	int result = (filled ? SDL_RenderFillRect(renderer, &rec) : SDL_RenderRect(renderer, &rec)) ? 0 : -1;

	if (result != 0)
	{
		LOG("Cannot draw quad to screen. SDL_RenderFillRect/SDL_RenderRect error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool Render::DrawLine(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool useCamera) const
{
	int scale = Engine::GetInstance().window->GetScale();
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	float X1 = static_cast<float>(useCamera ? camera.x + x1 * scale : x1 * scale);
	float Y1 = static_cast<float>(useCamera ? camera.y + y1 * scale : y1 * scale);
	float X2 = static_cast<float>(useCamera ? camera.x + x2 * scale : x2 * scale);
	float Y2 = static_cast<float>(useCamera ? camera.y + y2 * scale : y2 * scale);

	return SDL_RenderLine(renderer, X1, Y1, X2, Y2);
}

bool Render::DrawCircle(int x, int y, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool useCamera) const
{
	int scale = Engine::GetInstance().window->GetScale();
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	SDL_FPoint points[360];
	float factor = static_cast<float>(M_PI) / 180.0f;

	float cx = static_cast<float>((useCamera ? camera.x : 0) + x * scale);
	float cy = static_cast<float>((useCamera ? camera.y : 0) + y * scale);

	for (int i = 0; i < 360; ++i)
	{
		points[i].x = cx + static_cast<float>(radius * cos(i * factor));
		points[i].y = cy + static_cast<float>(radius * sin(i * factor));
	}

	SDL_RenderPoints(renderer, points, 360);
	return true;
}


void Render::DrawGlyph(char c, int x, int y, int scale, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool useCamera) const
{
	static const Uint8 SP[7] = { 0,0,0,0,0,0,0 };
	static const Uint8 A_[7] = { 0x0E,0x11,0x11,0x1F,0x11,0x11,0x11 };
	static const Uint8 B_[7] = { 0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E };
	static const Uint8 C_[7] = { 0x0E,0x11,0x10,0x10,0x10,0x11,0x0E };
	static const Uint8 D_[7] = { 0x1C,0x12,0x11,0x11,0x11,0x12,0x1C };
	static const Uint8 E_[7] = { 0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F };
	static const Uint8 F_[7] = { 0x1F,0x10,0x10,0x1E,0x10,0x10,0x10 };
	static const Uint8 G_[7] = { 0x0E,0x11,0x10,0x17,0x11,0x11,0x0E };
	static const Uint8 H_[7] = { 0x11,0x11,0x11,0x1F,0x11,0x11,0x11 };
	static const Uint8 I_[7] = { 0x0E,0x04,0x04,0x04,0x04,0x04,0x0E };
	static const Uint8 J_[7] = { 0x07,0x02,0x02,0x02,0x12,0x12,0x0C };
	static const Uint8 K_[7] = { 0x11,0x12,0x14,0x18,0x14,0x12,0x11 };
	static const Uint8 L_[7] = { 0x10,0x10,0x10,0x10,0x10,0x10,0x1F };
	static const Uint8 M_[7] = { 0x11,0x1B,0x15,0x11,0x11,0x11,0x11 };
	static const Uint8 N_[7] = { 0x11,0x19,0x15,0x13,0x11,0x11,0x11 };
	static const Uint8 O_[7] = { 0x0E,0x11,0x11,0x11,0x11,0x11,0x0E };
	static const Uint8 P_[7] = { 0x1E,0x11,0x11,0x1E,0x10,0x10,0x10 };
	static const Uint8 Q_[7] = { 0x0E,0x11,0x11,0x11,0x15,0x12,0x0D };
	static const Uint8 R_[7] = { 0x1E,0x11,0x11,0x1E,0x14,0x12,0x11 };
	static const Uint8 S_[7] = { 0x0F,0x10,0x10,0x0E,0x01,0x01,0x1E };
	static const Uint8 T_[7] = { 0x1F,0x04,0x04,0x04,0x04,0x04,0x04 };
	static const Uint8 U_[7] = { 0x11,0x11,0x11,0x11,0x11,0x11,0x0E };
	static const Uint8 V_[7] = { 0x11,0x11,0x11,0x11,0x11,0x0A,0x04 };
	static const Uint8 W_[7] = { 0x11,0x11,0x11,0x15,0x15,0x1B,0x11 };
	static const Uint8 X_[7] = { 0x11,0x11,0x0A,0x04,0x0A,0x11,0x11 };
	static const Uint8 Y_[7] = { 0x11,0x11,0x0A,0x04,0x04,0x04,0x04 };
	static const Uint8 Z_[7] = { 0x1F,0x01,0x02,0x04,0x08,0x10,0x1F };

	static const Uint8 _0[7] = { 0x0E,0x11,0x13,0x15,0x19,0x11,0x0E };
	static const Uint8 _1[7] = { 0x04,0x0C,0x04,0x04,0x04,0x04,0x0E };
	static const Uint8 _2[7] = { 0x0E,0x11,0x01,0x06,0x08,0x10,0x1F };
	static const Uint8 _3[7] = { 0x1F,0x02,0x04,0x02,0x01,0x11,0x0E };
	static const Uint8 _4[7] = { 0x02,0x06,0x0A,0x12,0x1F,0x02,0x02 };
	static const Uint8 _5[7] = { 0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E };
	static const Uint8 _6[7] = { 0x06,0x08,0x10,0x1E,0x11,0x11,0x0E };
	static const Uint8 _7[7] = { 0x1F,0x01,0x02,0x04,0x08,0x08,0x08 };
	static const Uint8 _8[7] = { 0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E };
	static const Uint8 _9[7] = { 0x0E,0x11,0x11,0x0F,0x01,0x02,0x0C };

	static const Uint8 LP[7] = { 0x02,0x04,0x08,0x08,0x08,0x04,0x02 };
	static const Uint8 RP[7] = { 0x08,0x04,0x02,0x02,0x02,0x04,0x08 };
	static const Uint8 SL[7] = { 0x01,0x01,0x02,0x04,0x08,0x10,0x10 };
	static const Uint8 MI[7] = { 0x00,0x00,0x00,0x1F,0x00,0x00,0x00 };
	static const Uint8 CO[7] = { 0x00,0x06,0x06,0x00,0x06,0x06,0x00 };
	static const Uint8 SPOT[7] = { 0x00,0x00,0x00,0x00,0x00,0x06,0x06 };

	const Uint8* rows = SP;
	unsigned char uc = static_cast<unsigned char>(c);
	char u = static_cast<char>(std::toupper(uc));

	switch (u)
	{
	case 'A': rows = A_; break; case 'B': rows = B_; break; case 'C': rows = C_; break;
	case 'D': rows = D_; break; case 'E': rows = E_; break; case 'F': rows = F_; break;
	case 'G': rows = G_; break; case 'H': rows = H_; break; case 'I': rows = I_; break;
	case 'J': rows = J_; break; case 'K': rows = K_; break; case 'L': rows = L_; break;
	case 'M': rows = M_; break; case 'N': rows = N_; break; case 'O': rows = O_; break;
	case 'P': rows = P_; break; case 'Q': rows = Q_; break; case 'R': rows = R_; break;
	case 'S': rows = S_; break; case 'T': rows = T_; break; case 'U': rows = U_; break;
	case 'V': rows = V_; break; case 'W': rows = W_; break; case 'X': rows = X_; break;
	case 'Y': rows = Y_; break; case 'Z': rows = Z_; break;

	case '0': rows = _0; break; case '1': rows = _1; break; case '2': rows = _2; break;
	case '3': rows = _3; break; case '4': rows = _4; break; case '5': rows = _5; break;
	case '6': rows = _6; break; case '7': rows = _7; break; case '8': rows = _8; break;
	case '9': rows = _9; break;

	case '(': rows = LP; break; case ')': rows = RP; break; case '/': rows = SL; break;
	case '-': rows = MI; break; case ':': rows = CO; break; case '.': rows = SPOT; break;

	default: rows = SP; break;
	}

	for (int j = 0; j < 7; ++j)
	{
		Uint8 row = rows[j];
		for (int i = 0; i < 5; ++i)
		{
			if (row & (1 << (4 - i)))
			{
				SDL_Rect pixel = { x + i * scale, y + j * scale, scale, scale };
				DrawRectangle(pixel, r, g, b, a, true, useCamera);
			}
		}
	}
}


void Render::DrawText(const char* text, int x, int y, int scale, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool useCamera) const
{
	int ox = x;
	for (const char* p = text; *p; ++p)
	{
		if (*p == '\n') { y += 8 * scale; x = ox; continue; }
		DrawGlyph(*p, x, y, scale, r, g, b, a, useCamera);
		x += 6 * scale;
	}
}
