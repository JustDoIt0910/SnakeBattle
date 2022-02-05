#include "snake.h"
#include "../server/server.h"
#include <iostream>
using namespace std;

Snake::Snake(vector<int> _pos, int _length, vector<vector<ObjInfo>>& M,
    int _size, OBJ_COLOR _color)
    :Object(SNAKE, _pos, _size, _color, M), length(_length)
{
    dirVec.resize(2);
    dirVec[0] = 0;
    dirVec[1] = -3;
    head = new SNode(pos[0], pos[1]);
    Map[pos[0]][pos[1]] = { id, size };
    tail = head;
    for(int j = 0; j < size / 3; j++)
        dq.push_back({pos[0], pos[1] + 3 * j});
    for(int i = 1; i < length; i++)
    {
        int y = pos[1] + i * size;
        tail->next = new SNode(pos[0], y);
        Map[pos[0]][y] = { id, size };
        tail = tail->next;
        if(i < length - 1)
            for(int j = 0; j < size / 3; j++)
                dq.push_back({pos[0], y + 3 * j});
    }
}

// Snake::Snake(vector<int> _pos, vector<vector<int>>& body, vector<vector<ObjInfo>>& M,
//     int _size, OBJ_COLOR _color)
//     :Object(SNAKE, _pos, _size, _color, M)
// {
//     dirVec.resize(2);
//     dirVec[0] = 0;
//     dirVec[1] = -3;
//     head = new SNode(pos[0], pos[1]);
//     tail = head;
//     for(int i = 0; i < body.size(); i++)
//     {
//         tail->next = new SNode(body[i][0], body[i][1]);
//         tail = tail->next;
//     }
// }

Snake::Snake(Snake* s): Object(SNAKE, s->pos, s->size, s->color, s->Map), 
    length(s->length), dirVec(s->dirVec)
{
    for(SNode* p = s->head; p != nullptr; p = p->next)
    {
        if(p == s->head)
        {
            head = new SNode(p->pos[0], p->pos[1]);
            tail = head;
        }
        else
        {
            tail->next = new SNode(p->pos[0], p->pos[1]);
            tail = tail->next;
        }
    }
}

Snake::~Snake()
{
    SNode* n = head;
    while(n != nullptr)
    {
        n = head->next;
        delete head;
        head = nullptr;
        head = n;
    }
}

void Snake::setID(int id)
{
    this->id = id;
}

int Snake::getID()
{
    return id;
}

void Snake::setDir(vector<char> _dirVec)
{
    dirVec = _dirVec;
}

int Snake::move()
{
    vector<int> sa = getSearchArea(10);
    for(int x = sa[0]; x <= sa[2]; x++)
        for(int y = sa[1]; y <= sa[3]; y++)
        {
            if(Map[x][y].id != -1 && Map[x][y].id != id)
                if(collision(x, y, Map[x][y].size))
                    return Map[x][y].id;
        }
    int i = 1;
    for(SNode* n = head->next; n != nullptr; n = n->next)
    {
       int index = (i++) * (size / 3) - 1;
       Map[n->pos[0]][n->pos[1]] = {-1, 0};
       n->pos[0] = dq.at(index)[0];
       n->pos[1] = dq.at(index)[1];
       Map[n->pos[0]][n->pos[1]] = {id, size};
    }
    Map[pos[0]][pos[1]] = {-1, 0};
    pos[0] = head->pos[0] = head->pos[0] + dirVec[0];
    pos[1] = head->pos[1] = head->pos[1] + dirVec[1];
    Map[pos[0]][pos[1]] = {id, size};
    dq.push_front({pos[0], pos[1]});
    dq.pop_back();
    return 0;
}

vector<vector<short>> Snake::getPositions()
{
    vector<vector<short>> poss;
    for(SNode* n = head; n != nullptr; n = n->next)
        poss.push_back(n->getPos());
    return poss;
}

vector<int> Snake::getSearchArea(int searchSize)
{   
    int cx = pos[0] + dirVec[0] * searchSize;
    int cy = pos[1] + dirVec[1] * searchSize;
    vector<int> rect;
    rect.push_back(cx - searchSize / 2);
    rect.push_back(cy - searchSize / 2);
    rect.push_back(cx + searchSize / 2);
    rect.push_back(cy + searchSize / 2);
    return rect;
}

bool Snake::collision(int x, int y, int _size)
{
    int dis = distance(x, y, pos[0], pos[1]);
    return dis <= _size + size;
}