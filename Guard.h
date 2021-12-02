#include <Windows.h>
#include <vector>
#include "Map.h"
#ifndef GUARD_H
#define GUARD_H

using namespace std;
class Guard
{
public:
	Guard(COORD pos, vector<COORD> patrol_wpts) 
		: position(pos), state(patrol), dir_facing(0), patrol_wpts(patrol_wpts){}

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
	};
	void Update()
	{
		switch (state)
		{
		case patrol: Patrol(); break;
		case chase: Chase(); break;
		}
	}
	bool InChase() { return state == chase; }
	void SetChase() 
	{ 
		state = chase; 
		turns_since_p_seen = 0;
		for (auto i : specific_waypoints)
			GetCell(i).is_path = false;
		for (auto i : view_cone_squares)
			GetCell(i).is_watched = false;
	}
private:
	void Patrol();
	void Chase();
	void AStar(COORD pos);
	void NewWaypoint();
	void Move();
	bool ScanForPlayer();
	void UpdateDirection(COORD new_pos);
	void CastRays();

	COORD position;
	char dir_facing;
	int state;
	int turns_since_p_seen = -1;
	vector<COORD> patrol_wpts;
	int current_ptrl_wpt = -1;
	vector<COORD> specific_waypoints;
	vector<COORD> view_cone_squares;
	int current_spc_wpt = -1;
	int stuck_counter = 0;
};

#endif // !GUARD_H