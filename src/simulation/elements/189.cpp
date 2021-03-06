#include "simulation/Elements.h"
#include "simulation/MULTIPPE_Update.h"
#include "simulation/sim-move-struct.h"
#include "Probability.h"

#ifdef LUACONSOLE
#include "lua/LuaScriptInterface.h"
#include "lua/LuaScriptHelper.h"
#endif

#ifdef _MSC_VER
#define __builtin_ctz msvc_ctz
#define __builtin_clz msvc_clz
#endif

#define saveWl cdcolour

//#TPT-Directive ElementClass Element_MULTIPP PT_E189 189
Element_MULTIPP::Element_MULTIPP()
{
	Identifier = "DEFAULT_PT_E189";
	Name = "E189";
	Colour = PIXPACK(0xFFB060);
	MenuVisible = 1;
	MenuSection = SC_SPECIAL;
// #if defined(DEBUG) || defined(SNAPSHOT)
	Enabled = 1;
// #else
//	Enabled = 0;
// #endif

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
	Hardness = 0; // Because ACID always not affects that!

	Weight = 100;

	DefaultProperties.temp = R_TEMP+0.0f	+273.15f;
	HeatConduct = 0;
	Description = "Experimental element. has multi-purpose.";

	Properties = TYPE_SOLID | PROP_NOSLOWDOWN;
	Properties2 = PROP_DEBUG_USE_TMP2 | PROP_CTYPE_SPEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &MULTIPPE_Update::update;
	Graphics = &MULTIPPE_Update::graphics;
	IconGenerator = &Element_MULTIPP::iconGen;
	// Notice: Exotic solid!
	// Properties without PROP_LIFE_DEC and PROP_LIFE_KILL_DEC, has reason.
}

//#TPT-Directive ElementHeader Element_MULTIPP static bool useDefaultPart
bool Element_MULTIPP::useDefaultPart = false;

//#TPT-Directive ElementHeader Element_MULTIPP static intptr_t * Arrow_keys
intptr_t * Element_MULTIPP::Arrow_keys = NULL; // Note: TPT uses SDL

//#TPT-Directive ElementHeader Element_MULTIPP static int maxPrior
int Element_MULTIPP::maxPrior = 0;

//#TPT-Directive ElementHeader Element_MULTIPP static int * EngineFrameStart
int * Element_MULTIPP::EngineFrameStart = NULL;

//#TPT-Directive ElementHeader Element_MULTIPP static void HSV2RGB(int ctype, int *r, int *g, int *b)
void Element_MULTIPP::HSV2RGB (int ctype, int *r, int *g, int *b)
{
	int ptmp = ctype;
	float tmpr, tmpg, tmpb;
	float hh, ss, vv, cc;
	int phue = (ptmp >> 16) % 0x600;
	if (phue < 0)
		phue += 0x600;
	hh = (float)phue / 256.0f;
	ss = (float)((ptmp >> 8) & 0xFF) / 255.0f;
	vv = (float)(ptmp & 0xFF);
	cc = vv * ss;
	int p_add = (int)(vv - cc);
	switch (phue >> 8)
	{
	case 0:
		tmpr = cc;
		tmpg = cc * hh;
		tmpb = 0.0f;
		break;
	case 1:
		tmpr = cc * (2.0f - hh);
		tmpg = cc;
		tmpb = 0.0f;
		break;
	case 2:
		tmpr = 0.0f;
		tmpg = cc;
		tmpb = cc * (hh - 2.0f);
		break;
	case 3:
		tmpr = 0.0f;
		tmpg = cc * (4.0f - hh);
		tmpb = cc;
		break;
	case 4:
		tmpr = cc * (hh - 4.0f);
		tmpg = 0.0f;
		tmpb = cc;
		break;
	case 5:
		tmpr = cc;
		tmpg = 0.0f;
		tmpb = cc * (6.0f - hh);
		break;
	}
	*r = (int)tmpr + p_add;
	*g = (int)tmpg + p_add;
	*b = (int)tmpb + p_add;
}


//#TPT-Directive ElementHeader Element_MULTIPP static VideoBuffer * iconGen(int, int, int)
VideoBuffer * Element_MULTIPP::iconGen(int toolID, int width, int height)
{
	VideoBuffer * newTexture = new VideoBuffer(width, height);
	
	for (int j = 0; j < height; j++)
	{
		int r = 100, g = 150, b = 50;
		int rd = 1, gd = -1, bd = -1;
		for (int i = 0; i < width; i++)
		{
			r += 15*rd;
			g += 15*gd;
			b += 15*bd;
			if (r > 200) rd = -1;
			if (g > 200) gd = -1;
			if (b > 200) bd = -1;
			if (r < 15) rd = 1;
			if (g < 15) gd = 1;
			if (b < 15) bd = 1;
			int rc = std::min(150, std::max(0, r));
			int gc = std::min(200, std::max(0, g));
			int bc = std::min(200, std::max(0, b));
			newTexture->SetPixel(i, j, rc, gc, bc, 255);
		}
	}
	
	return newTexture;
}

//#TPT-Directive ElementHeader Element_MULTIPP static void setFilter(Simulation* sim, int x, int y, int a, int w)
void Element_MULTIPP::setFilter(Simulation* sim, int x, int y, int a, int w)
{
	int r, ix, iy;
	ix = sim->portal_rx[a];
	iy = sim->portal_ry[a];

	setFilter(sim, x, y, ix, iy, w);
}

//#TPT-Directive ElementHeader Element_MULTIPP static void setFilter(Simulation* sim, int x, int y, int rx, int ry, int w)
void Element_MULTIPP::setFilter(Simulation* sim, int x, int y, int rx, int ry, int w)
{
	while (x >= 0 && y >= 0 && x < XRES && y < YRES)
	{
		unsigned int r = sim->pmap[y][x];
		if (TYP(r) != PT_FILT) break;
		sim->parts[ID(r)].ctype = w;
		x += rx; y += ry;
	}
}

//#TPT-Directive ElementHeader Element_MULTIPP static int interactDir(Simulation* sim, const void *mov_, bool is_phot, Particle* part_phot, Particle* part_other)
int Element_MULTIPP::interactDir(Simulation* sim, const void *mov_, bool is_phot, Particle* part_phot, Particle* part_other) // photons direction/type changer
{
	int t = -1;
	const Simulation_move & mov = *(const Simulation_move *)mov_;

	int rtmp = part_other->tmp, rtmp2 = part_other->tmp2, rct = part_other->ctype;
	int ctype, r1, r2, r3, temp;
	float rvx, rvy, rvx2, rvy2, rdif, multiplier = 1.0f;
	long long int lsb;
	signed char arr1[4] = {1,0,-1,0};
	signed char arr2[4] = {0,1,0,-1};
	if (rtmp)
	{
		int x, y;
		rvx = (float)rtmp2 / 1000.0f;
		rvy = (float)part_other->tmp3 / 1000.0f;
		switch (rtmp)
		{
		case 1:
			part_phot->vx = rvx;
			part_phot->vy = rvy;
			break;
		case 2:
			part_phot->vx += rvx;
			part_phot->vy += rvy;
			break;
		case 3:
			rvx2 = part_phot->vx;
			rvy2 = part_phot->vy;
			part_phot->vx = rvx2 * rvx - rvy2 * rvy;
			part_phot->vy = rvx2 * rvy + rvy2 * rvx;
			break;
		case 4:
			rvx2 = rvx * (M_PI / 180.0f);
			rdif = hypotf(part_phot->vx, part_phot->vy);
			if (rtmp & 0x100)
			{
				rvy2 = atan2f(part_phot->vy, part_phot->vx);
				rvx2 = rvx2 - rvy2;
			}
			part_phot->vx = rdif * cosf(rvx2);
			part_phot->vy = rdif * sinf(rvx2);
			break;
		case 5: // FILT wavelength changer (check 8 directions)
			x = (int)(part_other->x+0.5f);
			y = (int)(part_other->y+0.5f);
			switch (rtmp2)
			{
			case 0: case 1:
				{
					float angle = atan2f(part_phot->vy, part_phot->vx);
					t = 0;
					int a = (int)(floor(angle * (4.0f / M_PI) + 3.5f));
					if (rtmp2 & 1)
						setFilter(sim, x, y, (a + 2) & 7, part_phot->ctype),
						setFilter(sim, x, y, (a + 6) & 7, part_phot->ctype);
					else
						setFilter(sim, x, y, a & 7, part_phot->ctype);
				}
				break;
			case 2: case 3: case 4: case 5:
			case 6: case 7: case 8: case 9:
				setFilter(sim, x, y, rtmp2 - 2, part_phot->ctype);
				break;
			case 10: case 11:
				setFilter(sim, x, y, 2 * rtmp2 - 19, part_phot->ctype);
				setFilter(sim, x, y, 2 * rtmp2 - 15, part_phot->ctype);
				break;
			}
			break;
		case 6:
			part_phot->ctype = TYP(rtmp2);
			if (part_phot->ctype == PT_PROT)
				part_phot->flags |= FLAG_SKIPCREATE;
			else if (part_phot->ctype == PT_BRAY)
				part_phot->tmp = 0;
			return PT_E195;
#ifdef LUACONSOLE
		case 7:
			if (is_phot && !luatpt_interactDirELEM(mov.i, ID(mov.r), part_phot->ctype, rct, rtmp2))
				return 0;
			break;
#endif
		}
		return t;
	}

	{
		int l, ri = ID(mov.r);

		const int mask = 0x3FFFFFFF;
		const static int rot[10] = {0,1,1,1,0,-1,-1,-1,0,1};
		switch (rtmp2)
		{
			case 1: // beam splitter (random)
				rvx = part_phot->vx;
				rvy = part_phot->vy;
				(rct != 1) && (rdif = rvx, rvx = rvy, rvy = -rdif);
				r1 = rand() % ((rct == 4) ? 3 : 2);
				rdif = (r1 & 1) ? 1.0 : -1.0;
				if (r1 != 2 && (r1 + 2) != rct)
					part_phot->vx = rdif * rvx,
					part_phot->vy = rdif * rvy;
				break;
			case 2: // photons diode output
				l = sim->pmap[mov.y][mov.x];
				if (l)
				{
					const Particle & lp = sim->parts[ID(l)];
					if (lp.type == part_other->type && lp.life == 17)	// photons diode input
						return 0;	// filtered out
				}
				break;
			case 5: // random "energy" particle
				part_phot->ctype = PMAP(1, 1);
				return PT_E195;
			case 6: // photons absorber
				if (rct > 1)
					part_other->ctype --;
				else if (rct == 1)
					sim->kill_part(ri);
				return 0;
			case 7: // PHOT->NEUT
				return PT_NEUT;
			case 8: // PHOT->ELEC
				return PT_ELEC;
			case 9: // PHOT->PROT
				// part_phot->tmp = 0;
				part_phot->tmp2 = 0;
				return PT_PROT;
			case 10: // PHOT->GRVT
				part_phot->tmp = rct;
				return PT_GRVT;
			case 11: // PHOT (tmp: a -> b)
				r1 = 1 << (rct >> 1);
				if (rct)
					part_phot->tmp &= ~r1;
				else
					part_phot->tmp |=  r1;
				break;
			case 12: // set PHOT's temp
				part_phot->temp = part_other->temp;
				break;
			case 13: // set PHOT life
				part_phot->life = rct;
				break;
			case 14: // PHOT life extender
				if (part_phot->life > 0)
				{
					part_phot->life += rct;
					if (part_phot->life <= 0)
						if (rct < 0)
							return 0;
						else
							part_phot->life = 0;
				}
				break;
			// case 15: reserved code
			case 16: // velocity to wavelength converter
				rvx = part_phot->vx;
				rvy = part_phot->vy;
				rdif = rvx * rvx + rvy * rvy;
				ctype = part_phot->ctype;
#if defined(__GNUC__) || defined(_MSVC_VER)
				r1 = __builtin_ctz(ctype | ~mask);
				r2 = 31 - __builtin_clz(ctype & mask);
#else
				r1 = 30; r2 = 0;
				for (i = 0; i < 30; i++)
					if (ctype & (1<<i))
						(i < r1) && (r1 = i), (i > r2) && (r2 = i);
#endif
				r3 = floor(10.009775f-4.32808512f*logf(rdif));
				if (r3 > 0)
					(r3 + r1 >= 30) && (r3 = 29 - r1), ctype <<= r3;
				else
					(r3 + r2 < 0) && (r3 = - r2), ctype >>= -r3;
				multiplier = expf((float)r3*0.11552453f);
				part_phot->vx = rvx * multiplier;
				part_phot->vy = rvy * multiplier;
				part_phot->ctype = ctype & mask;
				break;
			case 17: // PHOT life multiplier
				if (part_phot->life > 0)
				{
					part_phot->life *= rct;
					if (part_phot->life < 0)
						part_phot->life = 0;
				}
				break;
			case 19: // beam splitter (switch)
				{
					int a, b = rct & 0x7, newrct, np;
					r1 = rct >> 7, r2 = ((rct & 0x7F) >> 3);

					if ((r2 >= 0 && r2 <= 8) || r2 == 10 || r2 == 12)
						part_phot->vx = r1 * rot[b+2],
						part_phot->vy = r1 * rot[b];

					if (!(r2 & 0x8))
						newrct = (rct & ~0x7F) | (b << 3) | (r2 & 7);
					else
					{
						newrct = rct;
						switch (r2)
						{
						case 8: newrct += 8; break;
						case 9: newrct -= 8; t = 0; break;
						case 10: sim->kill_part(ri); return t;
						case 11:
							a = floor( ( atan2f(rot[b], rot[b+2]) - atan2f(part_phot->vy, part_phot->vx) ) * (4.0 / M_PI) );
							if ((a + 3) & 2)
								break;
							createPhotonsWithVelocity(sim, mov.i, (int)(part_other->x+0.5f), (int)(part_other->y+0.5f), r1 * rot[b+2], r1 * rot[b]);
							break;
						case 13: t = 0;
						case 12:
							newrct = 0; part_other->tmp = 7; part_other->tmp2 = 0; break;
						}
					}
					part_other->ctype = newrct;
				}
				break;
			case 20: // conditional photon absorber
				if (rct >= 0x8 && rct <= 0x17)
				{
					static int ktable[8] = {
						FLAG_DIRCH_MARK_HK, FLAG_DIRCH_MARK_TVOID_K, FLAG_DIRCH_MARK_K, FLAG_DIRCH_MARK_HK,
						FLAG_DIRCH_MARK_VK, FLAG_DIRCH_MARK_TVOID_K, FLAG_DIRCH_MARK_VK, FLAG_DIRCH_MARK_K
					};
					int &c = sim->DIRCHInteractCount;
					int &s = sim->DIRCHInteractSize;
					int *t = sim->DIRCHInteractTable;
					if (c >= s)
					{
						s = (t == NULL) ? 64 : (s * 2);
						if ((t = (int*)realloc(t, s * sizeof(int))) != NULL);
							sim->DIRCHInteractTable = t;
					}
					if (t != NULL)
					{
						int f = sim->parts[ri].flags;
						t[c++] = mov.i;
						bool d = fabsf(part_phot->vx) > fabsf(part_phot->vy);
						int omsk = 0;

						if (rct >= 0xC)
							part_phot->vx = arr1[rct & 3],
							part_phot->vy = arr2[rct & 3];

						switch (rct >> 2)
						{
						case 2:
							omsk = ktable[(rct & 3) + (d ? 4 : 0)];
							break;
						case 3:
							omsk = f & (FLAG_DIRCH_MARK_HV) ? (FLAG_DIRCH_MARK_K) : 0;
							break;
						case 4:
							omsk = FLAG_DIRCH_MARK_TVOID_K;
							break;
						case 5:
							(f & (FLAG_DIRCH_MARK_HV)) ? (
								t = 0,
								(sim->parts[ri].saveWl != part_phot->ctype) &&
									(omsk |= FLAG_DIRCH_MARK_K)) :
								(sim->parts[ri].saveWl = part_phot->ctype);
							break;
						}
						
						omsk |= d ? FLAG_DIRCH_MARK_H : FLAG_DIRCH_MARK_V;
						sim->parts[ri].flags = f | omsk;
					}
				}
				else if (rot[(rct+2)&7] * part_phot->vx +
					     rot[rct&7] * part_phot->vy <= 0)
					return 0;
				break;
			case 21: // skip movement for N frame
				// part_phot->flags |= FLAG_SKIPMOVE;
				part_phot->tmp <<= 30;
				part_phot->tmp |= part_phot->ctype;
				part_phot->ctype = PMAP(1, 2);
				part_phot->tmp2 = rct;
				return PT_E195;
			case 22:
				part_phot->tmp = part_phot->ctype;
				part_phot->tmp2 = rct;
				part_phot->ctype = PMAP(1, 3);
				return PT_E195;
			case 23:
				if (rct <= 0)
					part_phot->tmp = * (int*) &(part_other->temp);
				else if (rct & 1)
					part_phot->tmp = * (int*) &(part_phot->temp);
				else
					part_phot->tmp = part_other->tmp3;
				part_phot->ctype = PMAP(1, 4);
				return PT_E195;
			case 26: // QRTZ-like scatter
				{
					float rr = part_other->ctype / 256.0f;
					float ra = rand() * (2.0f * M_PI) / (RAND_MAX + 1.0f);
					part_phot->vx = rr*cosf(ra);
					part_phot->vy = rr*sinf(ra);
				}
				break;
			case 27: // PHOT duplicator/splitter
			{
				float tvx = part_phot->vx, tvy = part_phot->vy;
				int np = createPhotonsWithVelocity(sim, mov.i, (int)(part_other->x+0.5f), (int)(part_other->y+0.5f), -tvy, tvx);
				if (np < 0)
					return 0;
				part_phot->vx = tvy;
				part_phot->vy = -tvx;
				break;
			}
		}
	}
	return t;
}

//#TPT-Directive ElementHeader Element_MULTIPP static int createPhotonsWithVelocity(Simulation* sim, int i, int x, int y, float vx, float vy)
int Element_MULTIPP::createPhotonsWithVelocity(Simulation* sim, int i, int x, int y, float vx, float vy)
{
	int &np = sim->pfree;
	if (np < 0)
		return -1;
	int nf = sim->parts[np].life;
	if (nf > sim->parts_lastActiveIndex)
		sim->parts_lastActiveIndex = nf;
	return createPhotonsWithVelocity2(sim, i, np, x, y, sim->parts[i].life, sim->parts[i].ctype, vx, vy), np;
}

//#TPT-Directive ElementHeader Element_MULTIPP static void createPhotonsWithVelocity2(Simulation* sim, int i, int np, int x, int y, int life, int ctype, float vx, float vy)
void Element_MULTIPP::createPhotonsWithVelocity2(Simulation* sim, int i, int np, int x, int y, int life, int ctype, float vx, float vy)
{
	Particle * parts = sim->parts;
	Particle * newphot = &(parts[np]);
	
	int t = parts[i].type;

	newphot->type = t;
	newphot->life = life;
	newphot->ctype = ctype;
	newphot->x = (float)x;
	newphot->y = (float)y;
	newphot->vx = vx;
	newphot->vy = vy;
	newphot->temp = parts[i].temp;
	newphot->tmp = parts[i].tmp;
	newphot->tmp2 = 0;
	newphot->tmp3 = 0;
	newphot->tmp4 = 0;
	newphot->flags = (np > i) ? FLAG_SKIPMOVE : 0;
	newphot->pavg[0] = sim->parts[np].pavg[1] = 0.0f;
	newphot->dcolour = 0;
	// cdcolour is ignored
	
	sim->photons[y][x] = PMAP(np, t);
	
	return;
}

//#TPT-Directive ElementHeader Element_MULTIPP static void duplicatePhotons(Simulation* sim, int i, int x, int y, Particle* part_phot, Particle* part_other)
void Element_MULTIPP::duplicatePhotons(Simulation* sim, int i, int x, int y, Particle* part_phot, Particle* part_other)
{
	int rtmp = part_other->tmp, np1 = sim->pfree, np2, max_np;

	if (!(rtmp & 0xFFFF) || np1 < 0)
		return; // fail

	bool split2 = (rtmp & 0x20000);
	np2 = sim->parts[np1].life;

	if (split2 && np2 < 0)
		return; // fail

	float rdif = (float)((((rtmp >> 8) ^ 0x80) & 0xFF) - 0x80) / 16.0f;
	float rvx = (float)(((rtmp ^ 0x08) & 0x0F) - 0x08) * rdif;
	float rvy = (float)((((rtmp >> 4) ^ 0x08) & 0x0F) - 0x08) * rdif;
	
	int nlife = part_other->tmp2;
	int nctype = part_other->ctype ? part_other->ctype : part_phot->ctype;

	max_np = (split2 && (np2 > np1)) ? np2 : np1;
	if (max_np > sim->parts_lastActiveIndex)
		sim->parts_lastActiveIndex = max_np;
	
	sim->pfree = sim->parts[split2?np2:np1].life;

	createPhotonsWithVelocity2(sim, i, np1, x, y, nlife, nctype, rvx, rvy);
	if (rtmp & 0x20000)
		createPhotonsWithVelocity2(sim, i, np2, x, y, nlife, nctype, -rvx, -rvy);
	if (rtmp & 0x10000)
	{
		sim->parts[np1].flags |= FLAG_PHOTDECO,
		sim->parts[np1].dcolour = part_phot->dcolour;
		if (rtmp & 0x20000)
			sim->parts[np2].flags |= FLAG_PHOTDECO,
			sim->parts[np2].dcolour = part_phot->dcolour;
	}
}

//#TPT-Directive ElementHeader Element_MULTIPP static bool DrawOn_(Simulation * sim, int i, int x, int y, int t, int v)
bool Element_MULTIPP::DrawOn_ (Simulation * sim, int i, int x, int y, int t, int v)
{
	Particle &part = sim->parts[i];
	switch (part.life)
	{
	case 10:
		if (t == PT_SPRK)
			sim->SimExtraFunc &= ~2;
		else if (t == PT_BIZR)
			sim->SimExtraFunc |= 0x400;
		else
			return false;
		return true;
	case 26:
		FloodButton(sim, i, x, y);
		return true;
	case 35:
		part.ctype = t;
		if ((t == PT_LIFE && v >= 0 && v < NGOL) || t == ELEM_MULTIPP)
			part.ctype |= PMAPID(v);
		return true;
	}
	return false;
}

//#TPT-Directive ElementHeader Element_MULTIPP static void FloodButton(Simulation *sim, int i, int x, int y)
void Element_MULTIPP::FloodButton(Simulation *sim, int i, int x, int y)
{
	int coord_stack_limit = XRES*YRES;
	unsigned short (*coord_stack)[2];
	int coord_stack_size = 0;
	int x1, x2;
	unsigned r;
	
	Particle * parts = sim->parts;
	int (*pmap)[XRES] = sim->pmap;
	
	if ((parts[i].type != ELEM_MULTIPP) || (parts[i].life != 26) || parts[i].tmp)
		return;

	coord_stack = new unsigned short[coord_stack_limit][2];
	coord_stack[coord_stack_size][0] = x;
	coord_stack[coord_stack_size][1] = y;
	coord_stack_size++;

	do
	{
		coord_stack_size--;
		x = coord_stack[coord_stack_size][0];
		y = coord_stack[coord_stack_size][1];
		x1 = x2 = x;
		
		// go left as far as possible
		while (x1 >= 1)
		{
			r = pmap[y][x1-1];
			if (!CHECK_EXTEL(r, 26))
			{
				break;
			}
			x1--;
		}
		// go right as far as possible
		while (x2 < XRES - 1)
		{
			r = pmap[y][x2+1];
			if (!CHECK_EXTEL(r, 26))
			{
				break;
			}
			x2++;
		}
		
		// fill span
		for (x=x1; x<=x2; x++)
		{
			r = pmap[y][x];
			parts[ID(r)].tmp = 8;
		}
		
		// add adjacent pixels to stack
		if (y >= 1)
			for (x=x1; x<=x2; x++)
			{
				r = pmap[y-1][x];
				if (CHECK_EXTEL(r, 26) && !parts[ID(r)].tmp)
				{
					coord_stack[coord_stack_size][0] = x;
					coord_stack[coord_stack_size][1] = y-1;
					coord_stack_size++;
					if (coord_stack_size>=coord_stack_limit)
					{
						delete[] coord_stack;
						return;
					}
				}
			}
		if (y < YRES-1)
			for (x=x1; x<=x2; x++)
			{
				r = pmap[y+1][x];
				if (CHECK_EXTEL(r, 26) && !parts[ID(r)].tmp)
				{
					coord_stack[coord_stack_size][0] = x;
					coord_stack[coord_stack_size][1] = y+1;
					coord_stack_size++;
					if (coord_stack_size>=coord_stack_limit)
					{
						delete[] coord_stack;
						return;
					}
				}
			}
		
	} while (coord_stack_size>0);
	delete[] coord_stack;
}

Element_MULTIPP::~Element_MULTIPP() {}
