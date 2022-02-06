#ifndef __FOOD_H__
#define __FOOD_H__
#include "object.h"

class Food: public Object
{
public:
    Food(vector<int> _pos, vector<vector<ObjInfo>>& M,
    int _size = FOOD_SIZE_DEFAULT, OBJ_COLOR _color = FOOD_COLOR_DEFAULT);
    Food(Food* f);
    ~Food();
    vector<vector<short>> getPositions();
};

#endif