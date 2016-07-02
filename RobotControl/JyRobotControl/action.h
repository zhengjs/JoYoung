#include "JoyoungRobot.h"
#include <vector>

typedef struct{
	MoveType moveType;
	int		 param1;
	int		 param2;
}ActionElement;

class Action{
public:
	void addActionElement(MoveType movetype, int param1, int param2);
	bool isCunrrentActionDone();

private:
	std::vector<ActionElement> mvActionQueue;
};