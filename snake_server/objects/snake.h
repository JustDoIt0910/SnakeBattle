#ifndef __SNAKE_H__
#define __SNAKE_H__
#include "object.h"
#include <deque>


struct SNode
{
    short pos[2];
    SNode* next;
    SNode(): next(nullptr){}
    SNode(int x, int y): next(nullptr)
    {
        pos[0] = x;
        pos[1] = y;
    }
    vector<short> getPos()
    {
        return {pos[0], pos[1]};
    }
};

class User;

class Snake : public Object
{
public:
    Snake(vector<int> _pos, int _length, vector<vector<ObjInfo>>& M,
    int _size = SNAKE_SIZE_DEFAULT, OBJ_COLOR _color = SNAKE_COLOR_DEFAULT);
    // Snake(vector<int> _pos, vector<vector<int>>& body, vector<vector<ObjInfo>>& M, 
    // int _size = SNAKE_SIZE_DEFAULT, OBJ_COLOR _color = SNAKE_COLOR_DEFAULT);
    Snake(Snake* s);
    ~Snake();
    void setID(int id);
    int getID();
    void setDir(vector<char> _dirVec);
    int move();
    void grow();
    vector<vector<short>> getPositions();

private:
    int id;
    SNode* head;
    SNode* tail;
    int length;
    vector<char> dirVec;
    deque<vector<int>> dq;

    vector<int> getSearchArea(int searchSize);
    bool collision(int x, int y, int _size);
};

#endif