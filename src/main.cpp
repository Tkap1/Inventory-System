#define m_debug


#include <stdint.h>
#include <math.h>

#include "raylib.h"

typedef uint8_t u8;
typedef uint32_t u32;
typedef u8 b8;

#ifdef m_debug

	#define WIN32_LEAN_AND_MEAN
	#define VC_EXTRALEAN
	#define NOGDICAPMASKS
	#define NOVIRTUALKEYCODES
	#define NOWINMESSAGES
	#define NOWINSTYLES
	#define NOSYSMETRICS
	#define NOMENUS
	#define NOICONS
	#define NOKEYSTATES
	#define NOSYSCOMMANDS
	#define NORASTEROPS
	#define NOSHOWWINDOW
	#define OEMRESOURCE
	#define NOATOM
	#define NOCLIPBOARD
	#define NOCOLOR
	#define NOCTLMGR
	#define NODRAWTEXT
	#define NOGDI
	#define NOKERNEL
	#define NOUSER
	#define NONLS
	#define NOMB
	#define NOMEMMGR
	#define NOMETAFILE
	#define NOMINMAX
	#define NOMSG
	#define NOOPENFILE
	#define NOSCROLL
	#define NOSERVICE
	#define NOSOUND
	#define NOTEXTMETRIC
	#define NOWH
	#define NOWINOFFSETS
	#define NOCOMM
	#define NOKANJI
	#define NOHELP
	#define NOPROFILER
	#define NODEFERWINDOWPOS
	#define NOMCX

	// @Note(tkap): Used for "IsDebuggerPresent"
	#include <windows.h>

	// @Note(tkap): Used for "printf"
	#include <stdio.h>
	#define m_assert(cond) if(!(cond)) \
	{ \
		printf("assertion failed: %s", #cond); \
		if(IsDebuggerPresent()) \
		{ \
			__debugbreak(); \
		} \
		else \
		{ \
			ExitProcess(1); \
		} \
	}
#else
	#define m_assert(cond)
#endif

#define m_assert_range(n, low, high) m_assert((n) >= (low) && (n) <= (high))
#define m_zero {}
#define m_null NULL
#define m_between(val, low, high) ((val) >= (low) && (val) <= (high))
#define m_at_least(x, y) ((y) < (x) ? (x) : (y))
#define m_at_most(x, y) ((y) > (x) ? (x) : (y))
#define m_clamp(val, x, y) m_at_most((y), m_at_least((x), (val)))


static constexpr Color hex3_to_color(u32 hex)
{
	Color result = m_zero;
	result.r = (u8)((hex & 0xff0000) >> 16);
	result.g = (u8)((hex & 0x00ff00) >> 8);
	result.b = (u8)((hex & 0x0000ff) >> 0);
	result.a = 255;
	return result;
}


#include "config.h"



struct s_v2i
{
	int x;
	int y;
};


struct s_item
{
	int width;
	int height;
	Texture2D texture;
};

struct s_mask_element
{
	s_v2i index;
	s_item* item;
};

struct s_add_item_result
{
	int collisions;
	s_v2i collision_indices[c_max_add_item_results];
	s_item* items[c_max_add_item_results];
};

struct s_inventory
{
	b8 occupied[c_inventory_height][c_inventory_width];
	s_item items[c_inventory_height][c_inventory_width];

	void get_mask(s_mask_element mask_out[c_inventory_height][c_inventory_width])
	{
		memset(mask_out, 0, sizeof(s_mask_element) * c_inventory_width * c_inventory_height);
		for(int y = 0; y < c_inventory_height; y++)
		{
			for(int x = 0; x < c_inventory_width; x++)
			{
				if(occupied[y][x])
				{
					s_item* item = &items[y][x];
					s_mask_element element = m_zero;
					element.index = {(int)x, (int)y};
					element.item = item;
					for(int height = 0; height < item->height; height++)
					{
						for(int width = 0; width < item->width; width++)
						{
							mask_out[y + height][x + width] = element;
						}
					}
				}
			}
		}
	}

	b8 add_item(s_item item_to_add)
	{

		m_assert_range(item_to_add.width, 1, c_inventory_width);
		m_assert_range(item_to_add.height, 1, c_inventory_height);

		s_mask_element mask[c_inventory_height][c_inventory_width];
		get_mask(mask);

		for(int x = 0; x < c_inventory_width; x++)
		{
			for(int y = 0; y < c_inventory_height; y++)
			{
				s_add_item_result result = add_item_at(item_to_add, x, y, mask);
				if(result.collisions == 0)
				{
					return true;
				}
			}
		}
		return false;
	}

	s_add_item_result add_item_at(s_item item_to_add, int x, int y, s_mask_element mask_in[c_inventory_height][c_inventory_width] = m_null)
	{
		m_assert_range(item_to_add.width, 1, c_inventory_width);
		m_assert_range(item_to_add.height, 1, c_inventory_height);
		m_assert_range(x, 0, c_inventory_width - 1);
		m_assert_range(y, 0, c_inventory_height - 1);

		s_add_item_result result = m_zero;

		s_mask_element temp_mask[c_inventory_height][c_inventory_width];
		s_mask_element* mask = m_null;
		if(mask_in)
		{
			mask = &mask_in[0][0];
		}
		else
		{
			get_mask(temp_mask);
			mask = &temp_mask[0][0];
		}

		bool item_fits = (y + item_to_add.height - 1) < c_inventory_height &&
			(x + item_to_add.width - 1) < c_inventory_width;

		if(item_fits)
		{
			for(int height = 0; height < item_to_add.height; height++)
			{
				for(int width = 0; width < item_to_add.width; width++)
				{
					s_mask_element* mask_element = mask + (x + width) + ((y + height) * c_inventory_width);
					if(mask_element->item)
					{
						b8 should_add_to_collisions = true;
						for(int i = 0; i < result.collisions; i++)
						{
							if(result.items[i] == mask_element->item)
							{
								should_add_to_collisions = false;
								break;
							}
						}
						if(should_add_to_collisions)
						{
							m_assert(result.collisions < c_max_add_item_results);
							result.items[result.collisions] = mask_element->item;
							result.collision_indices[result.collisions++] = mask_element->index;
						}
					}
				}
			}
		}
		else
		{
			result.collisions = c_max_add_item_results;
		}

		if(result.collisions == 0)
		{
			items[y][x] = item_to_add;
			occupied[y][x] = true;
		}
		return result;
	}

	void remove_item_at(int x, int y)
	{
		m_assert(occupied[y][x]);
		occupied[y][x] = false;
	}
};

static b8 mouse_collides_rect(Vector2 mouse, Vector2 pos, Vector2 size);

int main()
{

	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
	SetTraceLogLevel(LOG_ERROR);
	InitWindow(c_window_width, c_window_height, "Inventory");

	Texture2D sword_texture = LoadTexture("assets/sword.png");
	Texture2D shield_texture = LoadTexture("assets/shield.png");
	Texture2D ring_texture = LoadTexture("assets/ring.png");
	Texture2D big_sword_texture = LoadTexture("assets/big_sword.png");
	SetTextureFilter(sword_texture, TEXTURE_FILTER_BILINEAR);
	SetTextureFilter(shield_texture, TEXTURE_FILTER_BILINEAR);
	SetTextureFilter(ring_texture, TEXTURE_FILTER_BILINEAR);
	SetTextureFilter(big_sword_texture, TEXTURE_FILTER_BILINEAR);

	s_inventory inventory = m_zero;

	s_item sword = m_zero;
	sword.texture = sword_texture;
	sword.width = 1;
	sword.height = 3;

	s_item shield = m_zero;
	shield.texture = shield_texture;
	shield.width = 2;
	shield.height = 2;

	s_item ring = m_zero;
	ring.texture = ring_texture;
	ring.width = 1;
	ring.height = 1;

	s_item big_sword = m_zero;
	big_sword.texture = big_sword_texture;
	big_sword.width = 2;
	big_sword.height = 4;

	inventory.add_item(sword);
	inventory.add_item(sword);
	inventory.add_item(shield);
	inventory.add_item(shield);
	inventory.add_item(shield);
	inventory.add_item(ring);
	inventory.add_item(big_sword);

	b8 has_picked_item = false;
	s_item picked_item = m_zero;
	s_v2i picked_item_offset = m_zero;

	while(!WindowShouldClose())
	{
		BeginDrawing();
		ClearBackground(c_background_color);

		b8 left_pressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

		static constexpr int full_c_inventory_slot_size = c_inventory_slot_size + c_inventory_slot_separation;

		Vector2 mouse = GetMousePosition();

		s_v2i mouse_inventory_index = {
			(int)round(-c_inventory_x + mouse.x) / full_c_inventory_slot_size,
			(int)round(-c_inventory_y + mouse.y) / full_c_inventory_slot_size
		};

		for(int y = 0; y < c_inventory_height; y++)
		{
			for(int x = 0; x < c_inventory_width; x++)
			{
				int x_pos = c_inventory_x + x * (c_inventory_slot_size + c_inventory_slot_separation);
				int y_pos = c_inventory_y + y * (c_inventory_slot_size + c_inventory_slot_separation);
				DrawRectangle(x_pos, y_pos, c_inventory_slot_size, c_inventory_slot_size, c_inventory_slot_color);
			}
		}

		// @Note(tkap): 2 passes because we can't cleanly make raylib sort by Z.
		// If we don't do this, some inventory slots will be drawn on top of the item textures
		for(int y = 0; y < c_inventory_height; y++)
		{
			for(int x = 0; x < c_inventory_width; x++)
			{
				Vector2 pos = {
					(float)(c_inventory_x + x * full_c_inventory_slot_size),
					(float)(c_inventory_y + y * full_c_inventory_slot_size)
				};

				if(inventory.occupied[y][x])
				{
					s_item* item = &inventory.items[y][x];

					Vector2 size = {
						(float)(full_c_inventory_slot_size * item->width),
						(float)(full_c_inventory_slot_size * item->height)
					};

					DrawRectangleV(pos, size, c_item_background_color);
					DrawTexturePro(
						item->texture,
						{0, 0, (float)item->texture.width, (float)item->texture.height},
						{pos.x, pos.y, size.x, size.y},
						{0, 0},
						0,
						WHITE
					);

					if(mouse_collides_rect(mouse, pos, size))
					{
						if(left_pressed)
						{
							if(!has_picked_item)
							{
								inventory.remove_item_at(x, y);
								left_pressed = false;
								has_picked_item = true;
								picked_item = *item;

								picked_item_offset.x = x - mouse_inventory_index.x;
								picked_item_offset.y = y - mouse_inventory_index.y;
							}
						}
					}
				}
			}
		}

		if(has_picked_item)
		{

			s_v2i mouse_inventory_index_with_offset = mouse_inventory_index;
			mouse_inventory_index_with_offset.x += picked_item_offset.x;
			mouse_inventory_index_with_offset.y += picked_item_offset.y;

			Vector2 pos = {
				(float)(c_inventory_x + mouse_inventory_index_with_offset.x * full_c_inventory_slot_size),
				(float)(c_inventory_y + mouse_inventory_index_with_offset.y * full_c_inventory_slot_size)
			};

			Vector2 size = {
				(float)(full_c_inventory_slot_size * picked_item.width),
				(float)(full_c_inventory_slot_size * picked_item.height)
			};

			DrawRectangleV(pos, size, c_item_background_color);
			DrawTexturePro(
				picked_item.texture,
				{0, 0, (float)picked_item.texture.width, (float)picked_item.texture.height},
				{pos.x, pos.y, size.x, size.y},
				{0, 0},
				0,
				c_picked_item_tint
			);

			if(left_pressed)
			{
				if(
					m_between(mouse_inventory_index_with_offset.x, 0, c_inventory_width - 1) &&
					m_between(mouse_inventory_index_with_offset.y, 0, c_inventory_height - 1)
				)
				{
					s_add_item_result result = inventory.add_item_at(picked_item, mouse_inventory_index_with_offset.x, mouse_inventory_index_with_offset.y);
					if(result.collisions == 0)
					{
						has_picked_item = false;
					}
					else if(result.collisions == 1)
					{
						s_item temp = *result.items[0];
						inventory.remove_item_at(result.collision_indices[0].x, result.collision_indices[0].y);
						inventory.add_item_at(picked_item, mouse_inventory_index_with_offset.x, mouse_inventory_index_with_offset.y);
						picked_item = temp;
						picked_item_offset.x = result.collision_indices[0].x - mouse_inventory_index_with_offset.x;
						picked_item_offset.y = result.collision_indices[0].y - mouse_inventory_index_with_offset.y;

						picked_item_offset.x = m_clamp(picked_item_offset.x, -picked_item.width + 1, 0);
						picked_item_offset.y = m_clamp(picked_item_offset.y, -picked_item.height + 1, 0);
					}
				}
			}
		}

		EndDrawing();
	}

	return 0;
}

static b8 mouse_collides_rect(Vector2 mouse, Vector2 pos, Vector2 size)
{
	return mouse.x >= pos.x && mouse.x <= pos.x + size.x &&
		mouse.y >= pos.y && mouse.y <= pos.y + size.y;
}
