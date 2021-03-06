#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_DTEC PT_DTEC 162
Element_DTEC::Element_DTEC()
{
	Identifier = "DEFAULT_PT_DTEC";
	Name = "DTEC";
	Colour = PIXPACK(0xFD9D18);
	MenuVisible = 1;
	MenuSection = SC_SENSOR;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.96f;
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

	DefaultProperties.tmp2 = 2;
	HeatConduct = 0;
	Description = "Detector, creates a spark when something with its ctype is nearby.";

	Properties = TYPE_SOLID;
	Properties2 = PROP_DRAWONCTYPE | PROP_DEBUG_USE_TMP2;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_DTEC::update;
}

//#TPT-Directive ElementHeader Element_DTEC static int update(UPDATE_FUNC_ARGS)
int Element_DTEC::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, rt, rd = parts[i].tmp2, pavg, tmp;
	int nx, ny, ntmp;
	if (rd > 25) parts[i].tmp2 = rd = 25;

	tmp = parts[i].tmp3 & 1;
	if (parts[i].life) // if ( tmp3 == 0 and detected ) or ( tmp3 == 1 and undetected )
	{
		parts[i].life = 0;
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					rt = TYP(r); r = ID(r);
					if (!sim->parts_avg_elec(i, r))
					{
						if ((sim->elements[rt].Properties&(PROP_CONDUCTS|PROP_INSULATED)) == PROP_CONDUCTS && parts[r].life==0)
						{
							parts[r].life = 4;
							parts[r].ctype = rt;
							sim->part_change_type(r,x+rx,y+ry,PT_SPRK);
						}
					}
				}
	}
	bool setFilt = false;
	int photonWl = 0;
	for (rx=-rd; rx<rd+1; rx++)
		for (ry=-rd; ry<rd+1; ry++)
			if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if(!r)
					r = sim->photons[y+ry][x+rx];
				if(!r)
					continue;
				if (TYP(r) == parts[i].ctype && (parts[i].ctype != PT_LIFE || parts[i].tmp == parts[ID(r)].ctype || !parts[i].tmp))
					parts[i].life = 1;
				if (TYP(r) == PT_PHOT || (TYP(r) == PT_BRAY && parts[ID(r)].tmp!=2))
				{
					setFilt = true;
					photonWl = parts[ID(r)].ctype;
				}
			}
	parts[i].life ^= tmp;
	if (setFilt)
	{
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					Element_MULTIPP::setFilter(sim, x+rx, y+ry, rx, ry, photonWl);
				}
	}
	return 0;
}



Element_DTEC::~Element_DTEC() {}
