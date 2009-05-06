#include <stdio.h>
#include <math.h>
#include <sys/param.h>

#include <cairo.h>

#include "tilerender.h"
#include "render_healpixes.h"
#include "cairoutils.h"
#include "healpix.h"
#include "healpix-utils.h"
#include "log.h"

int render_healpixes(cairo_t* cairo, render_args_t* args) {
	int i, j, nside;
	il* hps;
	int hp;
	double ra, dec;
	double xyz[3];
	double r;
	double corners[] = { 0,0, 0,1, 1,1, 1,0 };

	nside = args->nside;
	if (nside == 0)
		nside = 1;
	ra  = pixel2ra (args->W/2.0, args);
	dec = pixel2dec(args->H/2.0, args);
	radecdeg2xyzarr(ra, dec, xyz);
	r = 0;
	// check distance to each corner of the image...
	for (j=0; j<4; j++) {
		double thisxyz[3];
		ra  = pixel2ra (corners[j*2+0] * args->W, args);
		dec = pixel2dec(corners[j*2+1] * args->H, args);
		radecdeg2xyzarr(ra, dec, thisxyz);
		r = MAX(r, sqrt(distsq(xyz, thisxyz, 3)));
	}

	hps = healpix_approx_rangesearch(xyz, r, nside, NULL);
	logmsg("Found %i healpixes within range.\n", il_size(hps));

	cairo_set_source_rgba(cairo, 1.0, 1.0, 1.0, 8.0);
	for (i=0; i<il_size(hps); i++) {
		hp = il_get(hps, i);
		for (j=0; j<4; j++) {
			double x, y;
			healpix_to_xyzarr(hp, nside, corners[j*2], corners[j*2+1], xyz);
			xyzarr2radecdeg(xyz, &ra, &dec);
			x = ra2pixelf(ra, args);
			y = dec2pixelf(dec, args);
			if (j == 0)
				cairo_move_to(cairo, x, y);
			else
				cairo_line_to(cairo, x, y);
		}
		cairo_close_path(cairo);
		cairo_stroke(cairo);
	}
	il_free(hps);

	return 0;
}