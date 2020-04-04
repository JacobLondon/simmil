#include <cassert>
#include <vector>
#include "modules.hpp"

using namespace pse;

// FIRST ITEM is DEFAULT
enum TileName {
    TILE_GRASS,
    TILE_BUILDING_TENT,
    TILE_HIGHLIGHT_MOUSE,
    TILE_ROAD_DIRT_STRAIGHT_NS,
    TILE_ROAD_DIRT_CORNER_NE,
    TILE_ROAD_DIRT_CORNER_SE,
    TILE_ROAD_DIRT_CORNER_SW,
    TILE_ROAD_DIRT_CORNER_NW,
    TILE_TEST_11,
    TILE_TEST_22,
    TILE_TEST_31,
    TILE_TEST_224,
    TILE_COUNT
};

constexpr int MAGIC_OFFSCREEN_MAX = 5;

struct TileDefinition {
    TileName name;
    ivec3 worldsize; // number of tiles x/y/z the tile takes up
    int id; // read-only, auto managed
    // Tile{TILE_GRASS, ivec2{1, 1}, ctx.load_image("assets/tile_grass.png")};
    TileDefinition();
    TileDefinition(TileName name, ivec3 size, int id);
};

TileDefinition::TileDefinition()
: name{TILE_GRASS}, worldsize{1, 1, 1}, id{-1}
{

}

TileDefinition::TileDefinition(TileName name, ivec3 size, int id)
: name{name}, worldsize{size.x, size.y, size.z}, id{id}
{

}

struct TileManager {
    TileDefinition *definition;
    /* Drawing can be complicated. The drawer has the coordinates
       of where to draw, but you can only draw once you get to
       the coordinates of the tile closest to the bottom of the
       screen. Keep track of who is where. */
    TileManager *drawer; // the one who can draw, closest to the top of the screen
    TileManager *commander; // the one who commands the drawer, closest to bottom of screen
    ivec2 world_coords;
    TileManager();
};

TileManager::TileManager()
: definition{nullptr}, drawer{nullptr}, commander{nullptr}, world_coords{}
{

}

struct WorldData {
    context& ctx;
    TileManager *world; // array of tile managers

    ivec2 screen_tilesize; // tile width and height in pixels
    ivec2 world_origin;
    ivec2 screen_offset;
    int world_height; // grids of the world tall
    int world_width; // grids of the world wide
    int world_hdiag;
    int world_vdiag;
    component *menu_component; // managed by the context
    component *world_component; // managed by the context
private:
    TileDefinition definitions[TILE_COUNT];
    TileDefinition *defaultdef = definitions;

public:
    WorldData(context& ctx, int height, int width);
    ~WorldData();
    void setup();
    void update();
    ivec2 world_to_screen(int wx, int wy);
    void tile_load(TileName name, int gridx, int gridy, int gridz, const char *path);
    void tile_place(TileName name, int wx, int wy);
    void tile_remove(int wx, int wy);
    void tile_draw(int wx, int wy);
};

WorldData::WorldData(context& ctx, int height, int width)
: ctx{ctx}, screen_tilesize{90, 45}, world_origin{width / 2, 1},
  world_height{height}, world_width{width}, world_hdiag{0}, world_vdiag{0},
  menu_component{nullptr}, world_component{nullptr}
{
    world_hdiag = fast_sqrtf(world_width * world_width * 2);
    world_vdiag = fast_sqrtf(world_height * world_height * 2);

    world = new TileManager[world_height * world_width];

    // order in which they appear
    world_component= new component{0, (int)(ctx.screen_height * 0.1), ctx.screen_width, (int)(ctx.screen_height * 0.9)};
    ctx.component_add(world_component);
    menu_component = new component{0, 0, ctx.screen_width, (int)(ctx.screen_height * 0.1)};
    ctx.component_add(menu_component);
}

WorldData::~WorldData()
{
    delete[] world;
}

void WorldData::tile_load(TileName name, int gridx, int gridy, int gridz, const char *path)
{
    definitions[name] = TileDefinition{name, ivec3{gridx, gridy, gridz}, ctx.load_image(path)};
}

/**
 * Place something on the the default tile or tile with a null definition.
 * Tiles CANNOT be placed on any other tile.
 * All tiles that the new tile takes up MUST be the default tile.
 */
void WorldData::tile_place(TileName name, int wx, int wy)
{
    if (wx < 0 || wx >= world_width || wy < 0 || wy >= world_height) {
        return;
    }

    // this cell already has something in it,
    // don't put something unless it is removed
    if (world[wy * world_width + wx].definition != nullptr &&
        world[wy * world_width + wx].definition != defaultdef)
    {
        return;
    }

    // can't place it, object reaches out of bounds
    if (wx + definitions[name].worldsize.x > world_width ||
        wy + definitions[name].worldsize.y > world_height)
    {
        return;
    }

    // ensure all tiles in the shape of the object are the default definition
    for (int i = wy; i < wy + definitions[name].worldsize.y; i++) {
        for (int j = wx; j < wx + definitions[name].worldsize.x; j++) {
            if (world[i * world_width + j].definition != nullptr &&
                world[i * world_width + j].definition != defaultdef)
            {
                return;
            }
        }
    }

    // assign drawer tile
    TileManager *drawer = &world[wy * world_width + wx];
    drawer->definition = &definitions[name];
    drawer->drawer = drawer;

    // assign all tiles in the shape of the object who they belong to and what they are
    for (int i = wy; i < wy + drawer->definition->worldsize.y; i++) {
        for (int j = wx; j < wx + drawer->definition->worldsize.x; j++) {
            world[i * world_width + j].definition = drawer->definition;
            world[i * world_width + j].drawer = drawer->drawer;
            world[i * world_width + j].commander = &world[(wy + drawer->definition->worldsize.y - 1) * world_width + (wx + drawer->definition->worldsize.x - 1)];
            world[i * world_width + j].world_coords = ivec2{j, i};
        }
    }

    // done
}

/**
 * If the tile is NOT the default tile,
 * replace it and all of the tiles in
 * its shape with the default tile
 */
void WorldData::tile_remove(int wx, int wy)
{
    if (wx < 0 || wx >= world_width || wy < 0 || wy >= world_height) {
        return;
    }

    // this cell has nothing to remove
    if (world[wy * world_width + wx].definition == defaultdef) {
        return;
    }

    // get the drawer tile
    TileManager *drawer = world[wy * world_width + wx].drawer;

    // clear all assigned tiles to default tile manually
    for (int i = drawer->world_coords.y; i < drawer->world_coords.y + drawer->definition->worldsize.y; i++) {
        for (int j = drawer->world_coords.x; j < drawer->world_coords.x + drawer->definition->worldsize.x; j++) {
            // tiles becomes its own drawer and commander
            world[i * world_width + j].drawer = &world[i * world_width + j];
            world[i * world_width + j].definition = defaultdef;
            world[i * world_width + j].commander = &world[i * world_width + j];
            world[i * world_width + j].world_coords = ivec2{j, i};
        }
    }
}

void WorldData::tile_draw(int wx, int wy)
{
    // ie. 2x2 -> 2
    const int world_largest_tile = 2;
    // is the tile even on the screen?
    ivec2 screen_coords = world_to_screen(wx, wy);
    if (screen_coords.x + world[wy * world_width + wx].definition->worldsize.x * screen_tilesize.x > world_component->x + world_largest_tile * world_component->w ||
        screen_coords.x < world_component->x - world_largest_tile * screen_tilesize.x ||
        screen_coords.y + world[wy * world_width + wx].definition->worldsize.y * screen_tilesize.y > world_component->y + world_largest_tile * world_component->h ||
        screen_coords.y < world_component->y - world_largest_tile * screen_tilesize.y)
    {
        return;
    }

    if (wx < 0 || wx >= world_width || wy < 0 || wy >= world_height) {
        return;
    }

    // don't draw the tile if it is not the commander
    if (&world[wy * world_width + wx] != world[wy * world_width + wx].commander) {
        return;
    }

    // The current coords are the lowest on screen, get the coords of the highest on screen
    // then calculate the offset of the top left corner of the tile image for drawing
    ivec2& dw = world[wy * world_width + wx].commander->drawer->world_coords;
    screen_coords = world_to_screen(dw.x, dw.y);
    int& id = world[dw.y * world_width + dw.x].definition->id;

    int& w  = screen_tilesize.x;
    int& h  = screen_tilesize.y;
    int& gx = world[dw.y * world_width + dw.x].definition->worldsize.x;
    int& gy = world[dw.y * world_width + dw.x].definition->worldsize.y;
    int& gz = world[dw.y * world_width + dw.x].definition->worldsize.z;

    int sx = screen_coords.x - w / 2 * gy + w / 2;
    int sy = screen_coords.y - (gz - 1) * h / 2;
    int sw = w / 2 * gy + w / 2 * gx;
    int sh = h / 2 * gx + h / 2 * gy + (gz - 1) * h / 2;

    ctx.draw_image(id, SDL_Rect{ sx, sy, sw, sh });
}

ivec2 WorldData::world_to_screen(int wx, int wy)
{
    return ivec2{
        (world_origin.x * screen_tilesize.x) + (wx - wy) * (screen_tilesize.x / 2) + screen_offset.x,
        (world_origin.y * screen_tilesize.y) + (wx + wy) * (screen_tilesize.y / 2) + screen_offset.y
    };
}

void WorldData::setup()
{
    // LOAD IMAGES IN THE SAME ORDER AS enum TileName
    tile_load(TILE_GRASS, 1, 1, 1, "assets/tile_grass_1x1.png");
    tile_load(TILE_BUILDING_TENT, 2, 2, 1, "assets/buildings_tent_2x2.png");
    tile_load(TILE_HIGHLIGHT_MOUSE, 1, 1, 1, "assets/hilite_mouse_1x1.png");
    tile_load(TILE_ROAD_DIRT_STRAIGHT_NS, 1, 1, 1, "assets/road_dirt_straight_ns_1x1.png");
    tile_load(TILE_ROAD_DIRT_CORNER_NE, 1, 1, 1, "assets/road_dirt_corner_ne_1x1.png");
    tile_load(TILE_ROAD_DIRT_CORNER_SE, 1, 1, 1, "assets/road_dirt_corner_se_1x1.png");
    tile_load(TILE_ROAD_DIRT_CORNER_SW, 1, 1, 1, "assets/road_dirt_corner_sw_1x1.png");
    tile_load(TILE_ROAD_DIRT_CORNER_NW, 1, 1, 1, "assets/road_dirt_corner_nw_1x1.png");
    tile_load(TILE_TEST_11, 1, 1, 1, "assets/test_1x1.png");
    tile_load(TILE_TEST_22, 2, 2, 1, "assets/test_2x2.png");
    tile_load(TILE_TEST_31, 3, 1, 1, "assets/test_3x1.png");
    tile_load(TILE_TEST_224, 2, 2, 4, "assets/test_2x2x4.png");

    // fill the world with the DEFAULT TILE DEFINITION
    // set the world_coords for each tile manager
    for (int i = 0; i < world_height; i++) {
        for (int j = 0; j < world_width; j++) {
            tile_place(defaultdef->name, j, i);
        }
    }

    tile_place(TILE_BUILDING_TENT, 2, 0);
    tile_place(TILE_ROAD_DIRT_STRAIGHT_NS, 3, 3);
    tile_place(TILE_ROAD_DIRT_STRAIGHT_NS, 3, 4);
    tile_place(TILE_ROAD_DIRT_STRAIGHT_NS, 3, 5);
    tile_place(TILE_ROAD_DIRT_CORNER_NE, 3, 6);
    tile_place(TILE_ROAD_DIRT_STRAIGHT_NS, 4, 6);
    tile_place(TILE_ROAD_DIRT_STRAIGHT_NS, 5, 6);
    tile_place(TILE_ROAD_DIRT_CORNER_SW, 6, 6);
    tile_place(TILE_ROAD_DIRT_STRAIGHT_NS, 6, 7);
    tile_place(TILE_ROAD_DIRT_STRAIGHT_NS, 6, 8);
    tile_place(TILE_ROAD_DIRT_STRAIGHT_NS, 6, 9);

    tile_place(TILE_ROAD_DIRT_CORNER_SE, 0, 0);
    tile_place(TILE_ROAD_DIRT_CORNER_SW, 1, 0);
    tile_place(TILE_ROAD_DIRT_CORNER_NW, 1, 1);
    tile_place(TILE_ROAD_DIRT_CORNER_NE, 0, 1);

    tile_place(TILE_TEST_11, 7, 2);
    tile_place(TILE_TEST_11, 7, 4);
    tile_place(TILE_TEST_224, 3, 7);
    tile_place(TILE_TEST_224, 1, 7);
    tile_place(TILE_TEST_31, 6, 1);

}

void WorldData::update()
{
    // TODO: Actually have a mouse thingy
    ivec2 mouse{ ctx.mouse.x, ctx.mouse.y };
    ivec2 mouse_cell{ (mouse.x - screen_offset.x) / screen_tilesize.x, (mouse.y - screen_offset.y) / screen_tilesize.y };
    ivec2 mouse_offset{ (mouse.x - screen_offset.x) % screen_tilesize.x, (mouse.y - screen_offset.y) % screen_tilesize.y };
    ivec2 mouse_selected{
        (mouse_cell.y - world_origin.y) + (mouse_cell.x - world_origin.x),
        (mouse_cell.y - world_origin.y) - (mouse_cell.x - world_origin.x)
    };

    // check if the mouse is in any of the corners of the current rectangle
    // selected to choose the correct world tile
    const ivec2 topleft{ 0, 0 };
    const ivec2 topmiddle{ screen_tilesize.x / 2, 0 };
    const ivec2 topright{ screen_tilesize.x, 0 };
    const ivec2 middleleft{ 0, screen_tilesize.y / 2 };
    const ivec2 middleright{ screen_tilesize.x, screen_tilesize.y / 2 };
    const ivec2 botleft{ 0, screen_tilesize.y };
    const ivec2 botmiddle{ screen_tilesize.x / 2, screen_tilesize.y };
    const ivec2 botright{ screen_tilesize.x, screen_tilesize.y };
    tri2 upleft{ topleft, topmiddle, middleleft };
    tri2 upright{ topmiddle, topright, middleright };
    tri2 downleft{ middleleft, botmiddle, botleft };
    tri2 downright{ botmiddle, middleright, botright };

    if (point_in_triangle(mouse_offset, upleft)) {
        mouse_selected.add(-1, 0);
    }
    else if (point_in_triangle(mouse_offset, upright)) {
        mouse_selected.add(0, -1);
    }
    else if (point_in_triangle(mouse_offset, downleft)) {
        mouse_selected.add(0, 1);
    }
    else if (point_in_triangle(mouse_offset, downright)) {
        mouse_selected.add(1, 0);
    }

    for (int wy = 0; wy < world_height; wy++) {
        for (int wx = 0; wx < world_width; wx++) {
            tile_draw(wx, wy);
        }
    }

    static int ox = 0;
    static int oy = 0;
    static int state = 0;

    // if another item is above, world component's focus is reset
    if (world_component->mouse_hovering) {
        ivec2 screen_selected_tile = world_to_screen(mouse_selected.x, mouse_selected.y);
        if (mouse_selected.x >= 0 && mouse_selected.x < world_width && mouse_selected.y >= 0 && mouse_selected.y < world_height) {
            ctx.draw_image(TILE_HIGHLIGHT_MOUSE, SDL_Rect{ screen_selected_tile.x, screen_selected_tile.y, screen_tilesize.x, screen_tilesize.y });
            //printf("Mouse: (%d, %d)\r", mouse_selected.x, mouse_selected.y);
        }

        const int xzoomamt = 14;
        const int yzoomamt = 7;
        const int xmaxzoom = 240;
        const int ymaxzoom = 120;
        const int xminzoom = 44;
        const int yminzoom = 22;
        //ivec2 screen_zoomed_selected_tile = world_to_screen(mouse_selected.x, mouse_selected.y);

        /* Zooming regularly occurs by stretching things further or closer to the origin. When the
           screen is looking at the world far from the origin, (45, 45), a "zoom out" of 1 pixel
           will cause the farthest tile to move 45 pixels and the (0, 0) coordinate to only be
           stretched by 1 tile. Calculate this delta relative to the tile the mouse is currently
           on, and where the tile moved to after the zoom. Change the world offset by those pixels. */

        if (ctx.mouse.scrollup) {
            if (screen_tilesize.x + xzoomamt < xmaxzoom && screen_tilesize.y + yzoomamt < ymaxzoom)
            {
                screen_tilesize.add(xzoomamt, yzoomamt);
            }
        }
        if (ctx.mouse.scrolldown) {
            if (screen_tilesize.x - xzoomamt > xminzoom && screen_tilesize.y - yzoomamt > yminzoom)
            {
                screen_tilesize.sub(xzoomamt, yzoomamt);
            }
        }
        ivec2 screen_zoomed_selected_tile = world_to_screen(mouse_selected.x, mouse_selected.y);
        screen_offset.add(screen_selected_tile.x - screen_zoomed_selected_tile.x, screen_selected_tile.y - screen_zoomed_selected_tile.y);

        switch (state) {
        case 0:
            if (ctx.mouse.lclick) {
                ox = mouse.x;
                oy = mouse.y;
                state = 1;
            }
            else {
                ox = 0;
                oy = 0;
            }
            break;
        case 1:
            if (ctx.mouse.lclick) {
                // in bounds!
                if (screen_offset.x + (mouse.x - ox) <  (world_hdiag / 2) * screen_tilesize.x &&
                    screen_offset.x + (mouse.x - ox) > -(world_hdiag) * screen_tilesize.x &&
                    screen_offset.y + (mouse.y - oy) <  (world_vdiag / 2) * screen_tilesize.y &&
                    screen_offset.y + (mouse.y - oy) > -(world_vdiag) * screen_tilesize.y)
                {
                    screen_offset.add((mouse.x - ox), (mouse.y - oy));
                    ox = mouse.x;
                    oy = mouse.y;
                }
            }
            else {
                ox = 0;
                oy = 0;
                state = 0;
            }
        }
    }

    if (ctx.check_key_invalidate(SDL_SCANCODE_SPACE)) {
        screen_offset = ivec2{0, 0};
    }
    printf("%d, %d\n", screen_offset.x, screen_offset.y);

    ctx.draw_rect(Red, SDL_Rect{world_component->x, world_component->y, world_component->w, world_component->h});
}

WorldData *world;

void simmil_setup(context& ctx)
{
    world = new WorldData(ctx, 30, 30);
    world->setup();
}

void simmil_update(context& ctx)
{
    (void)ctx;
    world->update();
}

void simmil_cleanup(pse::context& ctx)
{
    (void)ctx;
    delete world;
}
