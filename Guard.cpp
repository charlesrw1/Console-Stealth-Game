#include "Guard.h"
#include <algorithm>
#include <cmath>
#include <assert.h>
#include <set>

struct Node
{
	Node(Vec2 pos, Node* parent = nullptr)
		: pos(pos), parent(parent) {}
	
	Vec2 pos;
	Node* parent;
	float g = 0;
	float h = 0;
	float f = 0;
};

Node* FindNode(Vec2 c1, const set<Node*>& nodes)
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
void Guard::AStar(Vec2 goal)
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
			Vec2 new_pos = current->pos += movement[i];
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
	if (current_spc_wpt < 0)
		return; //crappy workaround, whatever
	if (GetCell(specific_waypoints.at(current_spc_wpt)).content != 'G') {
		GetCell(position).content = ' ';
		GetCell(position).is_path = false;
		GetCell(position).is_watched = false;

		UpdateDirection(specific_waypoints.at(current_spc_wpt));
		position = specific_waypoints.at(current_spc_wpt);
		GetCell(position).content = 'G';
		current_spc_wpt--;
	}
	else {
		stuck_counter++;
		if (stuck_counter > 2) {
			CleanUp();
			NewWaypoint();
			stuck_counter = 0;
		}
	}
}
bool Guard::BackFacing(Vec2 pos) const
{
	return (position + movement[dir_facing]*-1)== pos;
}
void Guard::UpdateDirection(Vec2 new_pos)
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
	//cast ray forwards, maybe more later
	Vec2 box_pos = position+movement[dir_facing];
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
			alert_exclamations.push_back(position + Vec2({0,-1 }));
			return true;
		}
	}
	return false;
}

//Guard states

void Guard::Patrol()
{
	//Guard is at waypoint, make new one
	if (current_spc_wpt == -1) {
		NewWaypoint();
		sentry_timer = 3;
		state = sentry;
	}
	Move();
	if (ScanForPlayer()) {
		SetChase();
		GFlags.alerted_flag = true;
	}
}
void Guard::Chase()
{
	turns_since_p_seen++;
	if (turns_since_p_seen > 10) {
		state = patrol;
		current_spc_wpt = -1;
		for (auto i : specific_waypoints)
			GetCell(i).is_path = false;

		if (ScanForPlayer())
			GFlags.alerted_flag = true;
	}
	else {
		for (auto i : specific_waypoints)
			GetCell(i).is_path = false;
		AStar(player_location);
		current_spc_wpt = specific_waypoints.size() - 2;
		if (specific_waypoints.size() < 3)
			GFlags.caught_flag = true;
		Move();
	}
}
void Guard::Sentry()
{
	sentry_timer--;
	if (sentry_timer < 0)
		state = patrol;
	else if (ScanForPlayer()) {
		SetChase();
		GFlags.alerted_flag = true;
	}
}