#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_PBCN PT_PBCN 153
Element_PBCN::Element_PBCN()
{
	Identifier = "DEFAULT_PT_PBCN";
	Name = "PBCN";
	Colour = PIXPACK(0x3B1D0A);
	MenuVisible = 1;
	MenuSection = SC_POWERED;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.97f;
	Loss = 0.50f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 12;

	Weight = 100;

	HeatConduct = 251;
	Description = "Powered breakable clone.";

	Properties = TYPE_SOLID|PROP_NOCTYPEDRAW | PROP_TRANSPARENT;
	Properties2 = PROP_CLONE;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_PBCN::update;
	Graphics = &Element_PBCN::graphics;
}

#define ADVECTION 0.1f

//#TPT-Directive ElementHeader Element_PBCN static int update(UPDATE_FUNC_ARGS)
int Element_PBCN::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, rt, np, ctype1 = parts[i].ctype;
	if (!parts[i].tmp2 && sim->pv[y/CELL][x/CELL]> sim->sim_max_pressure)
		parts[i].tmp2 = rand()%40+80;
	if (parts[i].tmp2)
	{
		parts[i].vx += ADVECTION*sim->vx[y/CELL][x/CELL];
		parts[i].vy += ADVECTION*sim->vy[y/CELL][x/CELL];
		parts[i].tmp2--;
		if(!parts[i].tmp2){
			sim->kill_part(i);
			return 1;
		}
	}
	if (ctype1 <= 0 || ctype1>=PT_NUM || !sim->elements[ctype1].Enabled || (ctype1 == PT_LIFE && (parts[i].tmp<0 || parts[i].tmp>=NGOL)))
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
					if (!(sim->elements[rt].Properties2 & PROP_CLONE) &&
					    rt!=PT_SPRK && rt!=PT_NSCN && rt!=PT_PSCN &&
					    rt!=PT_STKM && rt!=PT_STKM2 && rt<PT_NUM)
					{
						parts[i].ctype = rt;
						if (rt==PT_LIFE || rt==PT_LAVA || (rt == PT_E195 && partsi(r).ctype > 0 && partsi(r).ctype < PT_NUM))
							parts[i].tmp = partsi(r).ctype;
					}
				}
	if (parts[i].life!=10)
	{
		if (parts[i].life>0)
			parts[i].life--;
	}
	else
	{
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if (TYP(r)==PT_PBCN)
					{
						if (partsi(r).life<10 && partsi(r).life>0)
							parts[i].life = 9;
						else if (partsi(r).life == 0)
							partsi(r).life = 10;
					}
				}
		if (parts[i].ctype>0 && parts[i].ctype<PT_NUM && sim->elements[parts[i].ctype].Enabled)
		{
			if (parts[i].ctype==PT_PHOT) {//create photons a different way
				for (rx=-1; rx<2; rx++)
					for (ry = -1; ry < 2; ry++)
						if (rx || ry)
						{
							int r = sim->create_part(-1, x + rx, y + ry, PT_PHOT);
							if (r != -1)
							{
								parts[r].vx = rx * 3;
								parts[r].vy = ry * 3;
								if (r>i)
								{
									// Make sure movement doesn't happen until next frame, to avoid gaps in the beams of photons produced
									parts[r].flags |= FLAG_SKIPMOVE;
								}
							}
						}
			}
			else if (ctype1 == PT_LIFE)//create life a different way
				for (rx=-1; rx<2; rx++)
					for (ry=-1; ry<2; ry++)
						sim->create_part(-1, x+rx, y+ry, PT_LIFE, parts[i].tmp);

			else if (ctype1 == PT_E195) // not different way
			{
				np = sim->create_part(-1, x+rand()%3-1, y+rand()%3-1, TYP(parts[i].ctype));
				if (np>=0)
					parts[np].ctype = TYP(parts[i].tmp);
			}

			else if (ctype1 != PT_LIGH || (rand()%30)==0)
			{
				np = sim->create_part(-1, x+rand()%3-1, y+rand()%3-1, TYP(parts[i].ctype));
				if (np>=0)
				{
					if (ctype1 == PT_LAVA && parts[i].tmp>0 && parts[i].tmp<PT_NUM && sim->elements[parts[i].tmp].HighTemperatureTransition==PT_LAVA)
						parts[np].ctype = parts[i].tmp;
					// else if (ctype1 == ELEM_MULTIPP) // failed
					//	parts[np].life = parts[i].tmp;
				}
			}
		}
	}

	return 0;
}


//#TPT-Directive ElementHeader Element_PBCN static int graphics(GRAPHICS_FUNC_ARGS)
int Element_PBCN::graphics(GRAPHICS_FUNC_ARGS)

{
	int lifemod = ((cpart->life>10?10:cpart->life)*10);
	*colr += lifemod;
	*colg += lifemod/2;
	return 0;
}


Element_PBCN::~Element_PBCN() {}
