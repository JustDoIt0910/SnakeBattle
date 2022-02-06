#include "snake.h"
#include "../server/server.h"
#include <iostream>
using namespace std;

Snake::Snake(vector<int> _pos, int _length, vector<vector<ObjInfo>>& M,
    int _size, OBJ_COLOR _color)
    :Object(SNAKE, _pos, _size, _color, M), length(_length)
{
    id = generateID();
    dirVec.resize(2);
    dirVec[0] = 0;
    dirVec[1] = -3;
    head = new SNode(pos[0], pos[1]);
    Map[pos[0]][pos[1]] = {id, size, SNAKE};
    tail = head;
    for(int j = 0; j < size / 3; j++)
        dq.push_back({pos[0], pos[1] + 3 * j});
    for(int i = 1; i < length; i++)
    {
        int y = pos[1] + i * size;
        tail->next = new SNode(pos[0], y);
        Map[pos[0]][y] = {id, size, SNAKE};
        tail = tail->next;
        for(int j = 0; j < size / 3; j++)
            dq.push_back({pos[0], y + 3 * j});
    }
    
}

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

void Snake::setDir(vector<char> _dirVec)
{
    dirVec = _dirVec;
}

vector<int>& Snake::move()
{
    collisions.clear();
    if((pos[0] + dirVec[0] * size / 6 < 0) || (pos[1] + dirVec[1] * size / 6 < 0) ||
        (pos[0] + dirVec[0] * size / 6 >= MAP_SIZE) || (pos[1] + dirVec[1] * size / 6 >= MAP_SIZE))
    {
        removeFromMap();
        collisions.push_back(-1);
        return collisions;
    }
    vector<int> sa = getSearchArea(12, SNAKE);
    for(int x = max(0, sa[0]); x <= min(MAP_SIZE - 1, sa[2]); x++)
        for(int y = max(0, sa[1]); y <= min(MAP_SIZE - 1, sa[3]); y++)
        {
            ObjInfo info = Map[x][y];
            if(info.id != 0 && info.type == SNAKE && info.id != id)
                if(collision(x, y, info.size))
                {
                    removeFromMap();
                    collisions.push_back(info.id);
                    return collisions;
                }         
        }
    sa = getSearchArea(0, FOOD);
    for(int x = max(0, sa[0]); x <= min(MAP_SIZE - 1, sa[2]); x++)
        for(int y = max(0, sa[1]); y <= min(MAP_SIZE - 1, sa[3]); y++)
        {
            ObjInfo info = Map[x][y];
            if(info.id != 0 && info.type == FOOD && info.id != id)
                if(collision(x, y, info.size))
                {
                    foodCnt = (info.size == FOOD_SIZE_DEFAULT) ? foodCnt + 1 : foodCnt + 2;
                    collisions.push_back(info.id);
                    Map[x][y] = {0, 0, NONE};
                }
        }
    int i = 1;
    for(SNode* n = head->next; n != nullptr; n = n->next)
    {
       int index = (i++) * (size / 3) - 1;
       Map[n->pos[0]][n->pos[1]] = {0, 0, NONE};
       n->pos[0] = dq.at(index)[0];
       n->pos[1] = dq.at(index)[1];
       Map[n->pos[0]][n->pos[1]] = {id, size, SNAKE};
    }
    Map[pos[0]][pos[1]] = {0, 0, NONE};
    pos[0] = head->pos[0] = head->pos[0] + dirVec[0];
    pos[1] = head->pos[1] = head->pos[1] + dirVec[1];
    Map[pos[0]][pos[1]] = {id, size, SNAKE};
    dq.push_front({pos[0], pos[1]});
    dq.pop_back();

    if(foodCnt >= 2)
    {
        foodCnt = 0;
        int index = i * (size / 3) - 1;
        int x = dq.at(index)[0];
        int y = dq.at(index)[1];
        tail->next = new SNode(x, y);
        tail = tail->next;
        for(int j = 1; j <= size / 3; j++)
            dq.push_back({x - j * dirVec[0], y - j * dirVec[1]});
    }
    return collisions;
}

vector<vector<short>> Snake::getPositions()
{
    vector<vector<short>> poss;
    for(SNode* n = head; n != nullptr; n = n->next)
        poss.push_back(n->getPos());
    return poss;
}

vector<int> Snake::getSearchArea(int searchSize, OBJ_TYPE obj)
{   
    vector<int> rect;
    if(obj == SNAKE)
    {
        int cx = pos[0] + dirVec[0] * searchSize / 2;
        int cy = pos[1] + dirVec[1] * searchSize / 2;
        rect.push_back(cx - searchSize / 2);
        rect.push_back(cy - searchSize / 2);
        rect.push_back(cx + searchSize / 2);
        rect.push_back(cy + searchSize / 2);
    }
    else
    {
        int r = size / (2 * 1.414);
        rect.push_back(pos[0] - r);
        rect.push_back(pos[1] - r);
        rect.push_back(pos[0] + r);
        rect.push_back(pos[1] + r);
    }
    return rect;
}

bool Snake::collision(int x, int y, int _size)
{
    int dis = distance(x, y, pos[0], pos[1]);
    return dis <= ((_size + size) / 2);
}

void Snake::removeFromMap()
{
    for(SNode* n = head; n != nullptr; n = n->next)
        Map[n->pos[0]][n->pos[1]] = {0, 0, NONE};
}

bool Snake::readyToMove()
{
    if(speedUpFlag)
        return true;
    cnt++;
    if(cnt == 4)
    {
        cnt = 0;
        return true;
    }
    return false;
}

void Snake::speedUp(bool flag)
{
    speedUpFlag = flag;
}