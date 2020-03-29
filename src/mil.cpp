#include <vector>
#include "modules.hpp"

using namespace pse;

struct Tiles {
    int grass = -1;
    int tent = -1;
};

struct WorldData {
    Context& ctx;
    std::vector<std::vector<int>> world{};
    IVec2 tile_size;
    IVec2 origin;
    Tiles tiles;
    size_t world_height;
    size_t world_width;

    WorldData(Context& ctx, size_t height, size_t width);
    void setup();
    void update();
    IVec2 to_screen(int wx, int wy)
    {
        return IVec2{
            (origin.x * tile_size.x) + (wx - wy) * (tile_size.x / 2),
            (origin.y * tile_size.y) + (wx + wy) * (tile_size.y / 2)
        };
    }
};

WorldData::WorldData(Context& ctx, size_t height, size_t width)
: ctx{ctx}, tile_size{90, 45}, origin{width / 2, 1}, world_height(height), world_width(width)
{

}

void WorldData::setup()
{
    tiles.grass = ctx.load_image("assets/tile_grass.png");
    tiles.tent = ctx.load_image("assets/buildings_tent.png");
    for (int i = 0; i < world_height; i++) {
        world.emplace_back(world_width, tiles.grass);
    }
}

void WorldData::update()
{
    IVec2 mouse{ ctx.mouse.x, ctx.mouse.y };
    IVec2 mouse_cell{ mouse.x / tile_size.x, mouse.y / tile_size.y };
    IVec2 mouse_offset{ mouse.x % tile_size.x, mouse.y % tile_size.y };
    IVec2 mouse_selected{
        (mouse_cell.y - origin.y) + (mouse_cell.x - origin.x),
        (mouse_cell.y - origin.y) - (mouse_cell.x - origin.x)
    };

    // check if the mouse is in any of the corners of the current rectangle
    // selected to choose the correct world tile
    const IVec2 topleft{ 0, 0 };
    const IVec2 topmiddle{ tile_size.x / 2, 0 };
    const IVec2 topright{ tile_size.x, 0 };
    const IVec2 middleleft{ 0, tile_size.y / 2 };
    const IVec2 middleright{ tile_size.x, tile_size.y / 2 };
    const IVec2 botleft{ 0, tile_size.y };
    const IVec2 botmiddle{ tile_size.x / 2, tile_size.y };
    const IVec2 botright{ tile_size.x, tile_size.y };
    Triangle2 upleft{ topleft, topmiddle, middleleft };
    Triangle2 upright{ topmiddle, topright, middleright };
    Triangle2 downleft{ middleleft, botmiddle, botleft };
    Triangle2 downright{ botmiddle, middleright, botright };

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
            IVec2 screen = to_screen(wx, wy);

            int& id = world[wy][wx];
            ctx.draw_image(id, SDL_Rect{ screen.x, screen.y, tile_size.x, tile_size.y });
        }
    }

    if (mouse_selected.x >= 0 && mouse_selected.x < world_width && mouse_selected.y >= 0 && mouse_selected.y < world_height) {
        IVec2 selected_screen = to_screen(mouse_selected.x, mouse_selected.y);
        ctx.draw_image(tiles.tent, SDL_Rect{ selected_screen.x, selected_screen.y, tile_size.x, tile_size.y });
    }

    //ctx.draw_rect(Red, SDL_Rect{ mouse_cell.x * tile_size.x, mouse_cell.y * tile_size.y, tile_size.x, tile_size.y });
}

WorldData *world;

void simmil_setup(Context& ctx)
{
    world = new WorldData(ctx, 10, 10);
    world->setup();
}

void simmil_update(Context& ctx)
{
    world->update();
}
