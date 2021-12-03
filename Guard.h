#include <vector>
#include "Map.h"
#include "Utilities.h"
#include <iostream>
#ifndef GUARD_H
#define GUARD_H

using namespace std;
class Guard
{
public:
	Guard(Vec2 pos, vector<Vec2> patrol_wpts) 
		: position(pos), state(patrol), dir_facing(0), patrol_wpts(patrol_wpts)
	{
		GetCell(pos).content = 'G';
		NewWaypoint();
	}
	~Guard() { 
		CleanUp(); 
	}

	enum Direction
	{
		EAST = 0,
		WEST = 1,
		SOUTH = 2,
		NORTH = 3,
	};
	enum State
	{
		patrol,
		chase,
		sentry,
	};
	void Update()
	{
		switch (state)
		{
		case patrol: Patrol(); break;
		case chase: Chase(); break;
		case sentry: Sentry(); break;
		}
	}
	bool InChase() { return state == chase; }
	void SetChase() 
	{ 
		state = chase; 
		turns_since_p_seen = 0;
		CleanUp();
	}
	bool BackFacing(Vec2 pos) const;
	Vec2 GetPos() { return position; }
private:
	void Patrol();
	void Chase();
	void Sentry();
	void AStar(Vec2 pos);
	void NewWaypoint();
	void Move();
	bool ScanForPlayer();
	void UpdateDirection(Vec2 new_pos);
	void CastRays();
	void CleanUp()
	{
		for (auto i : specific_waypoints)
			GetCell(i).is_path = false;
		for (auto i : view_cone_squares)
			GetCell(i).is_watched = false;
		GetCell(patrol_wpts.at(current_ptrl_wpt)).is_target = false;
	}

	Vec2 position;
	char dir_facing;
	int state;
	int turns_since_p_seen = -1;
	int sentry_timer = 0;
	vector<Vec2> patrol_wpts;
	int current_ptrl_wpt = -1;
	vector<Vec2> specific_waypoints;
	vector<Vec2> view_cone_squares;
	int current_spc_wpt = -1;
	int stuck_counter = 0;
	Vec2 movement[4] = { {1,0},{-1,0},{0,1},{0,-1} };
};

#endif // !GUARD_H