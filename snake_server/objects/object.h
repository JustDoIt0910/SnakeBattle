#ifndef __OBJECT_H__
#define __OBJECT_H__
#include <vector>
#include <assert.h>
#include <math.h>
#define SNAKE_SIZE_DEFAULT 12
#define FOOD_SIZE_DEFAULT 3
using namespace std;

enum OBJ_COLOR
{
    RED = 0,
    YELLOW,
    GREEN,
    BLUE,
    BLACK,
    GREY
};

#define SNAKE_COLOR_DEFAULT RED
#define FOOD_COLOR_DEFAULT BLACK

enum OBJ_TYPE
{
    SNAKE = 0,
    FOOD
};

struct ObjInfo;

class Object
{
public:
    Object(OBJ_TYPE _type, vector<int> _pos, int _size, OBJ_COLOR _color, vector<vector<ObjInfo>>& M); 
    ~Object(){}
    vector<int> Pos();
    int Size();
    OBJ_COLOR Color();
    OBJ_TYPE Type();
    static int generateID();
    virtual vector<vector<short>> getPositions() = 0;

protected:
    vector<int> pos;
    int size;
    OBJ_TYPE type;
    OBJ_COLOR color;
    vector<vector<ObjInfo>>& Map;
};

double distance(int x1, int y1, int x2, int y2);

#endif