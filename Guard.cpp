#include "Guard.h"
#include <algorithm>
#include <cmath>
#include <assert.h>
#include <set>
#include <iostream>

struct Node
{
	Node(COORD pos, Node* parent = nullptr)
		: pos(pos), parent(parent) {}
	
	COORD pos;
	Node* parent;
	float g = 0;
	float h = 0;
	float f = 0;
};
bool operator != (COORD c1, COORD c2)
{
	return !((c1.X == c2.X) && (c1.Y == c2.Y));
}
bool operator == (COORD c1, COORD c2)
{
	return (c1.X == c2.X) && (c1.Y == c2.Y);
}
COORD operator += (COORD c1, COORD c2)
{
	return { short(c1.X + c2.X), short(c1.Y + c2.Y) };
}
COORD operator + (COORD c1, COORD c2)
{
	return { short(c1.X + c2.X), short(c1.Y + c2.Y) };
}
float Distance(COORD c1, COORD c2)
{
	return pow(c1.X + c2.X, 2) + pow(c1.Y + c2.Y, 2);
}

Node* FindNode(COORD c1, const set<Node*>& nodes)
{
	Node* result = nullptr;
	for (auto i : nodes) {
		if (i->pos == c1) {
			result = i;
			break;
		}
	}
	return result;
}
COORD movement[4] = { {1,0},{-1,0},{0,1},{0,-1} };
void Guard::AStar(COORD goal)
{
	int counter = 0;
	set<Node*> open_list;
	set<Node*> closed_list;
	open_list.insert(new Node(position));
	Node* current = nullptr;
	do 
	{
		assert(open_list.size() > 0);
		/*
		sort(open_list.begin(), open_list.end(), [](Node* a, Node* b)
			{
				return a->f > b->f;
			});
		current = open_list.back();
		open_list.pop_back();
		closed_list.push_back(current);
		*/
		auto current_it = open_list.begin();
		current = *open_list.begin();
		for (auto it = open_list.begin(); it != open_list.end(); it++)
		{
			auto node = *it;
			if (node->f < current->f)
			{
				current = node;
				current_it = it;
			}
		}
		closed_list.insert(current);
		open_list.erase(current_it);
		if (current->pos == goal)
			break;
		for (int i = 0; i < 4; i++)
		{
			COORD new_pos = current->pos += movement[i];
			if (CheckCollision(new_pos) || 
				FindNode(new_pos, closed_list))
				continue;
			float g = current->g + Distance(current->pos, new_pos);
			float h = Distance(goal, new_pos);
			int f = g + h;
			Node* find = FindNode(new_pos, open_list);
			if (find == nullptr)
			{
				find = new Node(new_pos, current);
				find->parent = current;
				find->g = g;
				find->h = h;
				find->f = f;
				open_list.insert(find);
			}
			else if (find->f > f)
			{
				find->parent = current;
				find->g = g;
				find->h = h;
				find->f = f;
			}
		}
	}while(!open_list.empty());
	//Add pathfind to waypoints
	specific_waypoints.clear();
	while (current)
	{
		specific_waypoints.push_back(current->pos);
		GetCell(current->pos).is_path = true;
		current = current->parent;
	}
	//Clean up
	for (auto i : open_list)
		delete i;
	for (auto i : closed_list)
		delete i;
}
void Guard::NewWaypoint()
{
	GetCell(position).is_target = false;
	current_ptrl_wpt = (current_ptrl_wpt + 1) % (patrol_wpts.size());
	AStar(patrol_wpts.at(current_ptrl_wpt));
	current_spc_wpt = specific_waypoints.size() - 2;
	GetCell(patrol_wpts.at(current_ptrl_wpt)).is_target = true;
}
void Guard::Move()
{
	//Guards waypoints may intersect, wait 2 turns if it resolves, else, go to next waypoint
	if (GetCell(specific_waypoints.at(current_spc_wpt)).content != 'G') {
		GetCell(position).content = ' ';
		GetCell(position).is_path = false;
		GetCell(position).is_watched = false;

		UpdateDirection(specific_waypoints.at(current_spc_wpt));
		position = specific_waypoints.at(current_spc_wpt);
		GetCell(position).content = 'G';
		current_spc_wpt--;
	}
	else
		stuck_counter++;
	if (stuck_counter > 2) {
		NewWaypoint();
		stuck_counter = 0;
	}
}
void Guard::Patrol()
{
	//Guard is at waypoint, make new one
	if (current_spc_wpt == -1)
		NewWaypoint();
	Move();
	if (ScanForPlayer())
		SetChase();
}
void Guard::UpdateDirection(COORD new_pos)
{
	dir_facing = 0;
	if (new_pos.Y < position.Y)
		dir_facing = NORTH;
	else if (new_pos.Y > position.Y)
		dir_facing = SOUTH;
	else if (new_pos.X < position.X)
		dir_facing = WEST;
	else if (new_pos.X > position.X)
		dir_facing = EAST;
}
void Guard::CastRays()
{
	//cast ray forwards
	COORD box_pos = position+movement[dir_facing];
	for (auto i : view_cone_squares)
		GetCell(i).is_watched = false;
	view_cone_squares.clear();
	for (int i = 0; i < 7; i++)
	{
		if (CheckCollision(box_pos))
			break;

		GetCell(box_pos).is_watched = true;
		view_cone_squares.push_back(box_pos);
		box_pos = box_pos + movement[dir_facing];
	}
}
bool Guard::ScanForPlayer()
{
	CastRays();
	if (position == player_location)
		return true;
	for (auto i : view_cone_squares) {
		if (i == player_location) {
			return true;
		}
	}
	return false;
}
void Guard::Chase()
{
	turns_since_p_seen++;
	if (turns_since_p_seen > 10) {
		state = patrol;
		current_spc_wpt = -1;
		for (auto i : specific_waypoints)
			GetCell(i).is_path = false;
	}
	else {
		for (auto i : specific_waypoints)
			GetCell(i).is_path = false;
		AStar(player_location);
		current_spc_wpt = specific_waypoints.size() - 2;
		if (specific_waypoints.size() < 3)
			game_running = false;
		Move();
	}
}