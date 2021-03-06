#include "ToolClasses.h"
#include "simulation/Simulation.h"
//#TPT-Directive ToolClass Tool_Displace TOOL_DISPLACE 8
Tool_Displace::Tool_Displace()
{
	Identifier = "DEFAULT_TOOL_DISPLACE";
	Name = "DSPL";
	Colour = PIXPACK(0xEE22EE);
	Description = "Displace tool.";
}

int Tool_Displace::Perform(Simulation * sim, Particle * cpart, int x, int y, int brushX, int brushY, float strength)
{
	int thisPart = sim->pmap[y][x];
	if(!thisPart)
		return 0;

	if(rand() % 100 != 0)
		return 0;

	int distance = (int)(std::pow(strength, .5f) * 10);

	if (!(distance & 1))
		distance ++;
	
	int newX = x + (rand() % distance) - (distance/2);
	int newY = y + (rand() % distance) - (distance/2);
	
	if(newX < 0 || newY < 0 || newX >= XRES || newY >= YRES)
		return 0;

	int thatPart = sim->pmap[newY][newX];

	if (!(sim->IsWallBlocking(newX, newY, TYP(thisPart)) || sim->IsWallBlocking(x, y, TYP(thatPart))))
	{
		sim->pmap[y][x] = thatPart;
		sim->pmap[newY][newX] = thisPart;
		sim->partsi(thisPart).x = newX;
		sim->partsi(thisPart).y = newY;
		if (thatPart)
		{
			sim->partsi(thatPart).x = x;
			sim->partsi(thatPart).y = y;
		}
		return 1;
	}
	
	return 0;
}

Tool_Displace::~Tool_Displace() {}
