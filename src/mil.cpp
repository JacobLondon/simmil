#include "modules.hpp"

using namespace pse;

constexpr int WORLDSIZ = 10;

struct {
    int tile_grass = -1;
    int buildings_tent = -1;
} Options;

Context *Ctx = nullptr;
int World[WORLDSIZ][WORLDSIZ] = { 0 };
IVec2 TileSize;
IVec2 Origin;

static IVec2 to_screen(int wx, int wy)
{
    return IVec2{
        (Origin.x * TileSize.x) + (wx - wy) * (TileSize.x / 2),
        (Origin.y * TileSize.y) + (wx + wy) * (TileSize.y / 2)
    };
}

void simmil_setup(Context& ctx)
{
    Options.tile_grass = ctx.load_image("assets/tile_grass.png");
    Options.buildings_tent = ctx.load_image("assets/buildings_tent.png");
    Ctx = &ctx;
    TileSize = IVec2{ 90, 45 };
    Origin = IVec2{ WORLDSIZ / 2, 1 };

    for (int i = 0; i < WORLDSIZ; i++) {
        for (int j = 0; j < WORLDSIZ; j++) {
            World[i][j] = Options.tile_grass;
        }
    }
    World[2][4] = Options.buildings_tent;
}

void simmil_update(Context& ctx)
{
    for (int wy = 0; wy < WORLDSIZ; wy++) {
        for (int wx = 0; wx < WORLDSIZ; wx++) {
            IVec2 screen_coord = to_screen(wx, wy);

            int& id = World[wy][wx];
            ctx.draw_image(id, SDL_Rect{ screen_coord.x, screen_coord.y, TileSize.x, TileSize.y });
        }
    }
}
