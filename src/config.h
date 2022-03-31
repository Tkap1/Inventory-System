
static constexpr int c_window_width = 800;
static constexpr int c_window_height = 600;
static constexpr int c_inventory_width = 12;
static constexpr int c_inventory_height = 5;

// @Note(tkap): This should be equal to the biggest item's width * height
static constexpr int c_max_add_item_results = 8;

static constexpr int slot_separation = 2;
static constexpr int slot_size = 30;
static constexpr int inventory_x = c_window_width / 2 - (c_inventory_width * (slot_separation + slot_size) / 2);
static constexpr int inventory_y = c_window_height / 2 - (c_inventory_height * (slot_separation + slot_size) / 2);

static constexpr Color c_inventory_slot_color = hex3_to_color(0x088897);
static constexpr Color c_background_color = RAYWHITE;
static constexpr Color c_picked_item_tint = {151, 151, 151, 255};
static constexpr Color c_item_background_color = {0, 0, 0, 100};