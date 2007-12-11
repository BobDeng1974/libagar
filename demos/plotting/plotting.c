/*
 * Copyright (c) 2007 Hypertriton, Inc. <http://hypertriton.com/>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * This program shows a practical use for the SC_Plotter widget. It computes
 * an optimal velocity profile using the squared sine algorithm, and plots
 * the derivatives.
 */

#include <agar/core.h>
#include <agar/gui.h>
#include <agar/sc.h>

#include <string.h>
#include <stdlib.h>
#include <math.h>

SC_Real L = 480.0;
SC_Real F = 1.0;
SC_Real Amax = 0.007;
SC_Real Jmax = 0.0001;
SC_Real uTs = 0.0;
SC_Real uTa = 0.0;
SC_Real uTo = 0.0;
SC_Plot *plVel, *plAcc, *plJerk;
SC_PlotLabel *plblCase;

/* Generated constants for squared sine */
SC_Real Aref;
SC_Real v, k;
SC_Real Ts, Ta, To;
SC_Real t1, t2, t3, t4, t5, t6, t7;
SC_Real v1, v2, v3;

/*
 * Compute the constants used in the squared sine algorithm, given the
 * parameters L (displacement), F, Amax and Jmax.
 */
static void
ComputeSquaredSineConstants(void)
{
	const char *which;

	/*
	 * Compute the shortest amount of time needed for the squared
	 * sine velocity profile for the given feedrate, under constraints
	 * of maximum acceleration and maximum jerk.
	 */
	if (F >= M_PI_2*((Amax*Amax)/Jmax) ) {
		/*
		 * Feedrate F is achievable by using maximum acceleration
		 * and maximum jerk.
		 */
		if (L >= ( (F*F)/Amax + M_PI_2*((Amax*F)/Jmax) )){
			/* Feedrate F is achievable at Amax acceleration. */
			which = "1.1";
			Ts = M_PI_2*(Amax/Jmax);
			Ta = (F/Amax) + Ts;
			To = L/F;
		} else if (L <  ( (F*F)/Amax + M_PI_2*((Amax*F)/Jmax) ) &&
	      		   L >= ( (M_PI*M_PI)/2.0) * (Amax*Amax*Amax) /
			                             (Jmax*Jmax) ) {
			/* Feedrate F is unachievable, Amax is achievable. */
			which = "1.2";
			Ts = M_PI_2*(Amax/Jmax);
			Ta = (( -M_PI*(Amax*Amax) +
			       sqrt((M_PI*M_PI)*(Amax*Amax*Amax*Amax) +
			            16.0*Amax*(Jmax*Jmax)*L)
			     ) /
			     ( 4.0*Jmax*Amax )) + Ts;
			To = Ta;
		} else if (L <= ( ((M_PI*M_PI)/2.0)*(Amax*Amax*Amax) /
		                                    (Jmax*Jmax) )) {
			/* Neither F nor Amax are achievable. */
			which = "1.3";
			Ts = cbrt( (M_PI*L)/(4.0*Jmax) );
			Ta = 2.0*Ts;
			To = Ta;
		} else {
			return;
		}
	} else if (F < M_PI_2*((Amax*Amax)/Jmax) ) {
		/*
		 * Feedrate F is achievable without using maximum
		 * acceleration.
		 */
		if (L >= sqrt( (2.0*M_PI*(F*F*F))/Jmax ) ) {
			/* F is achievable, Amax is unachievable. */
			which = "2.1";
			Ts = sqrt( (M_PI*F)/(2.0*Jmax) );
			Ta = 2.0*Ts;
			To = L/F;
		} else if (L < sqrt( (2.0*M_PI*(F*F*F))/Jmax )) {
			/* Neither F nor Amax are achievable. */
			which = "2.2";
			Ts = cbrt( (M_PI*L)/(4.0*Jmax) );
			Ta = 2.0*Ts;
			To = Ta;
		} else {
			return;
		}
	}
	if (uTs != 0.0) { Ts = uTs; }
	if (uTa != 0.0) { Ta = uTa; }
	if (uTo != 0.0) { To = uTo; }

	k = M_PI/(2.0*Ts);
	Aref = L/(Ta-Ts)/To;
	v1 = (Aref/2.0) * (Ts - sin(2.0*k*Ts)/(2.0*k) );
	v2 = Aref*(Ta - 2.0*Ts) + v1;
	v3 = v1+v2;

	t1 = Ts;
	t2 = Ta-Ts;
	t3 = Ta;
	t4 = To;
	t5 = To+Ts;
	t6 = To+Ta-Ts;
	t7 = To+Ta;
	
	SC_PlotLabelSetText(plVel, plblCase,  "case %s", which);
	SC_PlotLabelReplace(plVel, SC_LABEL_X, (unsigned)(Ts*L/t7), 0, "Ts");
	SC_PlotLabelReplace(plVel, SC_LABEL_X, (unsigned)(Ta*L/t7), 0, "Ta");
	SC_PlotLabelReplace(plVel, SC_LABEL_X, (unsigned)(To*L/t7), 0, "To");
}

SC_Real
SquaredSineStep(SC_Real t)
{
	if (t <= t1) {
		v = (Aref/2.0)*( t - sin(2.0*k*t)/(2.0*k) );
	} else if (t <= t2) {
		v = Aref*(t-t1) + v1;
	} else if (t <= t3) {
		v = (Aref/2.0)*( (t-t2) + sin(2.0*k*(t-t2))/(2.0*k) ) + v2;
	} else if (t <= t4) {
		v = v3;
	} else if (t <= t5) {
		v = -(Aref/2.0)*( (t-t4) - sin(2.0*k*(t-t4))/(2.0*k) ) + v3;
	} else if (t <= t6) {
		v = -Aref*(t-t5) + v2;
	} else if (t <= t7) {
		v = -(Aref/2.0)*( (t-t6) + sin(2.0*k*(t-t6))/(2.0*k) ) + v1;
	} else {
		v = 0.0;
	}
	return (v);
}

static void
GeneratePlot(AG_Event *event)
{
	SC_Plotter *plt = AG_PTR(1);
	SC_Real t;

	/* Clear the current plot data. */
	SC_PlotClear(plVel);
	SC_PlotClear(plAcc);
	SC_PlotClear(plJerk);

	/*
	 * Compute the data. SC_PlotterUpdate() will compute the derivatives
	 * for us.
	 */
	ComputeSquaredSineConstants();
	for (t = 0.0; t < L; t += 1.0) {
		SC_PlotReal(plVel, SquaredSineStep(t*t7/L));
		SC_PlotterUpdate(plt);
	}
}

int
main(int argc, char *argv[])
{
	AG_Window *win;
	SC_Plotter *plt;
	AG_Pane *pane;
	AG_Numerical *num;
	AG_Box *box;
	int i;

	if (AG_InitCore("plotting-demo", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	if (AG_InitVideo(640, 480, 32, AG_VIDEO_OPENGL) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (-1);
	}
	SC_InitSubsystem();
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	AG_BindGlobalKey(SDLK_F8, KMOD_NONE, AG_ViewCapture);

	/* Create a new window. */
	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "SC_Plotter example");

	pane = AG_PaneNew(win, AG_PANE_HORIZ, AG_PANE_EXPAND);
	{
		/* Create our plotter widget */
		plt = SC_PlotterNew(pane->div[1], SC_PLOTTER_EXPAND);

		/*
		 * Create the velocity plot item. This is what our algorithm
		 * computes.
		 */
		plVel = SC_PlotNew(plt, SC_PLOT_LINEAR);
		SC_PlotSetLabel(plVel, "m/s");
		SC_PlotSetYoffs(plVel, -45);
		SC_PlotSetScale(plVel, 0.0, 90.0);

		/* Create a label we will use to show the "case". */
		plblCase = SC_PlotLabelNew(plVel, SC_LABEL_OVERLAY, 0, 16, "-");

		/* Plot the derivative of the velocity (the acceleration). */
		plAcc = SC_PlotFromDerivative(plt, SC_PLOT_LINEAR, plVel);
		SC_PlotSetLabel(plAcc, "m/s^2");
		SC_PlotSetYoffs(plAcc, 42);
		SC_PlotSetScale(plAcc, 0.0, 3000.0);

		/* Plot the derivative of the acceleration (the jerk). */
		plJerk = SC_PlotFromDerivative(plt, SC_PLOT_LINEAR, plAcc);
		SC_PlotSetLabel(plJerk, "m/s^3");
		SC_PlotSetScale(plJerk, 0.0, 100.0);
		SC_PlotSetScale(plJerk, 0.0, 180000.0);
		SC_PlotSetYoffs(plJerk, 122);
	}

	/* Allow the user to play with the parameters. */
	box = AG_BoxNew(pane->div[0], AG_BOX_VERT, AG_BOX_EXPAND);
	{
		struct {
			const char *name;
			SC_Real *f;
			double incr;
		} param[7] = {
			{ "Jmax: ",	&Jmax,	0.00001 },
			{ "Amax: ",	&Amax,	0.0005 },
			{ "F: ",	&F,	0.01 },
			{ "L: ",	&L,	10.0 },
			{ "Ts tweak: ",	&uTs,	1.0 },
			{ "Ta tweak: ",	&uTa,	1.0 },
			{ "To tweak: ",	&uTo,	1.0 },
		};

		for (i = 0; i < 7; i++) {
			num = AG_NumericalNew(box, 0, NULL, param[i].name);
			AG_WidgetBindDouble(num, "value", param[i].f);
			AG_NumericalSetIncrement(num, param[i].incr);
			AG_ExpandHoriz(num);
			AG_SetEvent(num, "numerical-changed",
			    GeneratePlot, "%p", plt);
		}

		AG_SeparatorNewHoriz(box);
		AG_LabelNewPolled(box, 0, "Aref: %F", &Aref);
		AG_LabelNewPolled(box, 0, "v1: %F", &v1);
		AG_LabelNewPolled(box, 0, "v2: %F", &v2);
		AG_LabelNewPolled(box, 0, "v3: %F", &v3);
		AG_LabelNewPolled(box, 0, "Ts: %F", &Ts);
		AG_LabelNewPolled(box, 0, "Ta: %F", &Ta);
		AG_LabelNewPolled(box, 0, "To: %F", &To);
		AG_ButtonAct(box, AG_BUTTON_HFILL, "Generate",
		    GeneratePlot, "%p", plt);
	}
	AG_SetEvent(win, "window-shown", GeneratePlot, "%p", plt);
	AG_WindowMaximize(win);
	AG_WindowShow(win);

	AG_EventLoop();
	AG_Destroy();
	return (0);
}
