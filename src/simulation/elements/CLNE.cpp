#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_CLNE PT_CLNE 9
Element_CLNE::Element_CLNE()
{
	Identifier = "DEFAULT_PT_CLNE";
	Name = "CLNE";
	Colour = PIXPACK(0xFFD010);
	MenuVisible = 1;
	MenuSection = SC_SPECIAL;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.90f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;

	Weight = 100;

	HeatConduct = 251;
	Description = "Clone. Duplicates any particles it touches.";

	Properties = TYPE_SOLID|PROP_DRAWONCTYPE|PROP_NOCTYPEDRAW | PROP_TRANSPARENT;
	Properties2 = PROP_CLONE | PROP_UNBREAKABLECLONE;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_CLNE::update;
}

//#TPT-Directive ElementHeader Element_CLNE static int update(UPDATE_FUNC_ARGS)
int Element_CLNE::update(UPDATE_FUNC_ARGS)
{
	int ctype1 = parts[i].ctype;
	if (ctype1 <= 0 || ctype1 >= PT_NUM || !sim->elements[ctype1].Enabled || (ctype1==PT_LIFE && (parts[i].tmp<0 || parts[i].tmp>=NGOL)))
	{
		int r, rx, ry, rt;
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (BOUNDS_CHECK)
				{
					r = sim->photons[y+ry][x+rx];
					if (!r)
						r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					rt = TYP(r);
					if (!(sim->elements[rt].Properties2 & PROP_CLONE)
						&& rt!=PT_STKM && rt!=PT_STKM2 && rt!=PT_E195 && rt<PT_NUM)
					{
						parts[i].ctype = rt;
						if (rt==PT_LIFE || rt==PT_LAVA)
							parts[i].tmp = partsi(r).ctype;
					}
				}
	}
	else {
		if (ctype1 == PT_LIFE) sim->create_part(-1, x+rand()%3-1, y+rand()%3-1, PT_LIFE, parts[i].tmp);
		else if (ctype1 != PT_LIGH || (rand()%30)==0)
		{
			int np = sim->create_part(-1, x+rand()%3-1, y+rand()%3-1, TYP(ctype1));
			if (np>=0)
			{
				if ((ctype1 == PT_LAVA && parts[i].tmp>0 && parts[i].tmp<PT_NUM && sim->elements[parts[i].tmp].HighTemperatureTransition==PT_LAVA) || ctype1 == PT_E195)
					parts[np].ctype = parts[i].tmp;
			}
		}
	}
	return 0;
}


Element_CLNE::~Element_CLNE() {}
