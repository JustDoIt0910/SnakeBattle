#include "object.h"

Object::Object(OBJ_TYPE _type, vector<int> _pos, int _size, OBJ_COLOR _color, vector<vector<ObjInfo>>& M): 
    type(_type), size(_size), color(_color), Map(M)
{ 
    assert(_pos.size() == 2);
    pos = _pos;
}

vector<int> Object::Pos() { return pos; }

int Object::Size() { return size; }

OBJ_COLOR Object::Color() { return color; }

OBJ_TYPE Object::Type() { return type; }

int Object::generateID()
{
    static int id = 0;
    id++;
    return id;
}

double distance(int x1, int y1, int x2, int y2)
{
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}