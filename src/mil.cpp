#include <cassert>
#include <vector>
#include "modules.hpp"

using namespace pse;

enum TileName {
    TILE_GRASS,
    BUILDING_TENT,
    TILE_COUNT
};

struct TileDefinition {
    TileName name;
    ivec2 size; // number of tiles wide/tall the tile takes up
    int id; // read-only, auto managed
    // Tile{TILE_GRASS, ivec2{1, 1}, ctx.load_image("assets/tile_grass.png")};
    TileDefinition();
    TileDefinition(TileName name, ivec2 size, int id);
};

TileDefinition::TileDefinition()
: name{TILE_GRASS}, size{1, 1}, id{-1}
{

}

TileDefinition::TileDefinition(TileName name, ivec2 size, int id)
: name{name}, size{size.x, size.y}, id{id}
{

}

struct TileManager {
    TileDefinition *definition;
    TileManager *owner;
    ivec2 world_coords;
    TileManager();
};

TileManager::TileManager()
: definition{nullptr}, owner{nullptr}, world_coords{}
{

}

struct WorldData {
    context& ctx;
    TileManager *world; // array of tile managers

    ivec2 tile_size; // tile width and height in pixels
    ivec2 world_origin;
    int world_height;
    int world_width;
private:
    TileDefinition definitions[TILE_COUNT];
    TileDefinition *defaultdef = definitions;

public:
    WorldData(context& ctx, int height, int width);
    ~WorldData();
    void setup();
    void update();
    ivec2 world_to_screen(int wx, int wy);
    void tile_load(TileName name, int gridwidth, int gridheight, const char *path);
    void tile_place(TileName name, int wx, int wy);
    void tile_remove(int wx, int wy);
    void tile_draw(int wx, int wy);
};

WorldData::WorldData(context& ctx, int height, int width)
: ctx{ctx}, tile_size{90, 45}, world_origin{width / 2, 1}, world_height(height), world_width(width)
{
    world = new TileManager[world_height * world_width];
}

WorldData::~WorldData()
{
    delete[] world;
}

void WorldData::tile_load(TileName name, int gridwidth, int gridheight, const char *path)
{
    definitions[name] = TileDefinition{name, ivec2{gridwidth, gridheight}, ctx.load_image(path)};
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
    if (wx + definitions[name].size.x >= world_width ||
        wy + definitions[name].size.y >= world_height)
    {
        return;
    }

    // ensure all tiles in the shape of the object are the default definition
    for (int i = wy; i < wy + definitions[name].size.y; i++) {
        for (int j = wx; j < wx + definitions[name].size.x; j++) {
            if (world[wy * world_width + wx].definition != nullptr &&
                world[i * world_width + j].definition != defaultdef)
            {
                return;
            }
        }
    }

    // assign owner tile
    TileManager *owner = &world[wy * world_width + wx];
    owner->definition = &definitions[name];
    owner->owner = owner;

    // assign all tiles in the shape of the object who they belong to and what they are
    for (int i = wy; i < wy + owner->definition->size.y; i++) {
        for (int j = wx; j < wx + owner->definition->size.x; j++) {
            world[i * world_width + j].definition = owner->definition;
            world[i * world_width + j].owner = owner->owner;
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

    // get the owner tile
    TileManager *owner = world[wy * world_width + wx].owner;

    // clear all assigned tiles to default tile manually
    for (int i = owner->world_coords.y; i < owner->world_coords.y + owner->definition->size.y; i++) {
        for (int j = owner->world_coords.x; j < owner->world_coords.x + owner->definition->size.x; j++) {
            // tiles becomes its own owner
            world[i * world_width + j].owner = &world[i * world_width + j];
            world[i * world_width + j].definition = defaultdef;
        }
    }
}

void WorldData::tile_draw(int wx, int wy)
{
    if (wx < 0 || wx >= world_width || wy < 0 || wy >= world_height) {
        return;
    }

    // don't draw the tile if it is not the owner
    if (&world[wy * world_width + wx] != world[wy * world_width + wx].owner) {
        return;
    }

    ivec2 screen_coords = world_to_screen(wx, wy);
    int& id = world[wy * world_width + wx].definition->id;
    int& w = tile_size.x;
    int& h = tile_size.y;
    int& gx = world[wy * world_width + wx].definition->size.x;
    int& gy = world[wy * world_width + wx].definition->size.y;

    int sx = screen_coords.x - w / 2 * gy;
    int sy = screen_coords.y;
    int sw = w / 2 * gy + w / 2 * gx;
    int sh = h / 2 * gx + h / 2 * gy;

    ctx.draw_image(id, SDL_Rect{ sx, sy, sw, sh });
}

ivec2 WorldData::world_to_screen(int wx, int wy)
{
    return ivec2{
        (world_origin.x * tile_size.x) + (wx - wy) * (tile_size.x / 2),
        (world_origin.y * tile_size.y) + (wx + wy) * (tile_size.y / 2)
    };
}

void WorldData::setup()
{
    // LOAD IMAGES IN THE SAME ORDER AS enum TileName
    tile_load(TILE_GRASS, 1, 1, "assets/tile_grass.png");
    tile_load(BUILDING_TENT, 2, 2, "assets/test2x2.png");

    // fill the world with the DEFAULT TILE DEFINITION
    // set the world_coords for each tile manager
    for (int i = 0; i < world_height; i++) {
        for (int j = 0; j < world_width; j++) {
            tile_place(defaultdef->name, j, i);
        }
    }

    
    tile_place(BUILDING_TENT, 2, 0);
    tile_place(BUILDING_TENT, 0, 1);
    tile_place(BUILDING_TENT, 2, 2);

}

void WorldData::update()
{
    ivec2 mouse{ ctx.mouse.x, ctx.mouse.y };
    ivec2 mouse_cell{ mouse.x / tile_size.x, mouse.y / tile_size.y };
    ivec2 mouse_offset{ mouse.x % tile_size.x, mouse.y % tile_size.y };
    ivec2 mouse_selected{
        (mouse_cell.y - world_origin.y) + (mouse_cell.x - world_origin.x),
        (mouse_cell.y - world_origin.y) - (mouse_cell.x - world_origin.x)
    };

    // check if the mouse is in any of the corners of the current rectangle
    // selected to choose the correct world tile
    const ivec2 topleft{ 0, 0 };
    const ivec2 topmiddle{ tile_size.x / 2, 0 };
    const ivec2 topright{ tile_size.x, 0 };
    const ivec2 middleleft{ 0, tile_size.y / 2 };
    const ivec2 middleright{ tile_size.x, tile_size.y / 2 };
    const ivec2 botleft{ 0, tile_size.y };
    const ivec2 botmiddle{ tile_size.x / 2, tile_size.y };
    const ivec2 botright{ tile_size.x, tile_size.y };
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

    /*for (int wy = 0; wy < world_height; wy++) {
        for (int wx = 0; wx < world_width; wx++) {
            tile_draw(wx, wy);
        }
    }*/

    for (int sum = 0; sum < world_width + world_height - 1; sum++) {
        for (int wx = 0; wx <= sum; wx++) {
            tile_draw(wx, sum - wx);
        }
    }

    if (mouse_selected.x >= 0 && mouse_selected.x < world_width && mouse_selected.y >= 0 && mouse_selected.y < world_height) {
        ivec2 selected_screen = world_to_screen(mouse_selected.x, mouse_selected.y);
        ctx.draw_image(BUILDING_TENT, SDL_Rect{ selected_screen.x - tile_size.x, selected_screen.y - tile_size.y, tile_size.x * 2, tile_size.y * 2 });
    }

    //ctx.draw_rect(Red, SDL_Rect{ mouse_cell.x * tile_size.x, mouse_cell.y * tile_size.y, tile_size.x, tile_size.y });
}

WorldData *world;

void simmil_setup(context& ctx)
{
    world = new WorldData(ctx, 10, 10);
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
