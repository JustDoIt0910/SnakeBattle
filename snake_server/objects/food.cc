#include "food.h"
#include "../server/server.h"

Food::Food(vector<int> _pos, vector<vector<ObjInfo>>& M,
int _size, OBJ_COLOR _color)
    :Object(FOOD, _pos, _size, _color, M)
{
    id = generateID();
    Map[pos[0]][pos[1]] = {id, size, FOOD};
}

Food::Food(Food* f): Object(FOOD, f->pos, f->size, f->color, f->Map){}

vector<vector<short>> Food::getPositions()
{
    vector<short> p = {(short)pos[0], (short)pos[1]};
    vector<vector<short>> poss = {p};
    return poss;
}

Food::~Food(){}