/* See LICENSE file for copyright and license details. */

#include "layouts.h"
#include "push.h"
#include "util.h"

//TODO: fix the ugly misalignment when gaps are enabled and there are an even
//      number of windows in both the master and stack
void
bstack(Monitor *m) {
	int w, h, mh, mx, tx, ty, tw, framecount;
	unsigned int i, n;
	Client *c;

	if (animated && clientcount() > 4)
		framecount = 4;
	else
		framecount = 7;

	for (n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++);
	if (n == 0)
		return;
	if (n > m->nmaster) {
		mh = m->nmaster ? m->mfact * m->wh: 0;
		tw = (m->ww - m->gappx) / (n - m->nmaster);
		ty = m->wy + mh;
	} else {
		mh = m->wh - m->gappx;
		tw = m->ww - m->gappx;
		ty = m->wy;
	}
	for (i = mx = 0, tx = m->wx, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++) {
		if (i < m->nmaster) {
			w = (m->ww - mx) / (MIN(n, m->nmaster) - i) - m->gappx;
			animateclient(c, m->wx + mx + m->gappx, m->wy + m->gappx, w - (2*c->bw) - m->gappx, mh - (2*c->bw), framecount, 0);
			mx += WIDTH(c) + m->gappx;
		} else {
			h = m->wh - mh - m->gappx;
			animateclient(c, tx + m->gappx, ty + (2*m->gappx), tw - (2*c->bw) - m->gappx, h - (2*c->bw) - (2*m->gappx), framecount, 0);
			if (tw != m->ww)
				tx += WIDTH(c) + m->gappx;
		}
	}
}


/*
 * Different ids for snapping positions
 *
 *    ##################################
 *    # 8             1              2 # 
 *    #                                # 
 *    #                                # 
 *    #                                # 
 *    # 7             9              3 # 
 *    #                                # 
 *    #                                # 
 *    # 6             5              4 # 
 *    ##################################
 *
 * */

void floatl(Monitor *m) {
    Client *c;
    int animatestore;
    animatestore = animated;
    animated = 0;
    for(c = m->clients; c; c = c->next) {
        if (!(ISVISIBLE(c)))
            continue;
        if (c->snapstatus)
            applysnap(c, m);
    }
    restack(selmon);
    if (selmon->sel)
        XRaiseWindow(dpy, selmon->sel->win);
    if (animatestore)
        animated = 1;
}



void
bstackhoriz(Monitor *m) {
	int w, mh, mx, tx, ty, th, framecount;
	unsigned int i, n;
	Client *c;

	if (animated && clientcount() > 4)
		framecount = 4;
	else
		framecount = 7;

	for (n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++);
	if (n == 0)
		return;
	if (n > m->nmaster) {
		mh = m->nmaster ? m->mfact * m->wh - m->gappx: 0;
		th = (m->wh - mh) / (n - m->nmaster);
		ty = m->wy + mh;
	} else {
		th = mh = m->wh - m->gappx;
		ty = m->wy - m->gappx;
	}
	for (i = mx = 0, tx = m->wx, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++) {
		if (i < m->nmaster) {
			w = (m->ww - mx) / (MIN(n, m->nmaster) - i);
			animateclient(c, m->wx + mx + m->gappx, m->wy + m->gappx, w - (2 * c->bw) - (2*m->gappx), mh - (2 * c->bw) - (2*m->gappx), framecount, 0);
			mx += WIDTH(c) + m->gappx;
		} else {
		animateclient(c, tx + m->gappx, ty, m->ww - (2 * c->bw) - (2*m->gappx), th - (2 * c->bw) - m->gappx, framecount, 0);
			if (th != m->wh)
				ty += HEIGHT(c) + m->gappx;
		}
	}
}

void
deck(Monitor *m)
{
	int dn;
	unsigned int i, n, h, mw, my;
	Client *c;

	for(n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++);
	if(n == 0)
		return;

	dn = n - m->nmaster;
	if(dn > 0) /* override layout symbol */
		snprintf(m->ltsymbol, sizeof m->ltsymbol, "D %d", dn);

	if(n > m->nmaster)
		mw = m->nmaster ? m->ww * m->mfact : 0;
	else
		mw = m->ww - m->gappx;
	for(i = 0, my = m->gappx, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++)
		if(i < m->nmaster) {
			h = (m->wh - my) / (MIN(n, m->nmaster) - i) - m->gappx;
			resize(c, m->wx + m->gappx, m->wy + my, mw - (2*c->bw) - m->gappx, h - (2*c->bw), False);
			my += HEIGHT(c) + m->gappx;
		}
		else
			resize(c, m->wx + mw + m->gappx, m->wy, m->ww - mw - (2*c->bw) - 2*m->gappx, m->wh - (2*c->bw), False);
}

void
grid(Monitor *m) {
	int i, n, rows, framecount;
	unsigned int cols;
	Client *c;

	if (m->clientcount <= 2 && m->mw > m->mh)
	{
		tile(m);
		return;
	}

	if (animated && clientcount() > 5)
		framecount = 3;
	else
		framecount = 6;
	for(n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next))
		n++;

	/* grid dimensions */
	for(rows = 0; rows <= n/2; rows++)
		if(rows*rows >= n)
			break;
	cols = (rows && (rows - 1) * rows >= n) ? rows - 1 : rows;

	/* window geoms (cell height/width) */
	int ch = m->wh / (rows ? rows : 1);
	int cw = m->ww / (cols ? cols : 1);
	for(i = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next)) {
		unsigned int cx = m->wx + (i / rows) * cw;
		unsigned int cy = m->wy + (i % rows) * ch;
		/* adjust height/width of last row/column's windows */
		int ah = ((i + 1) % rows == 0) ? m->wh - ch * rows : 0;
		unsigned int aw = (i >= rows * (cols - 1)) ? m->ww - cw * cols : 0;
		animateclient(c, cx + m->gappx, cy + m->gappx, cw - 2*c->bw + aw - m->gappx, ch - 2*c->bw + ah - m->gappx, framecount, 0);
		i++;
	}
}

// overlay all clients on top of each other
void
monocle(Monitor *m)
{
	unsigned int n = 0;
	Client *c;

	if (animated && selmon->sel)
		XRaiseWindow(dpy, selmon->sel->win);

	for (c = m->clients; c; c = c->next)
		if (ISVISIBLE(c))
			n++;
	if (n > 0) /* override layout symbol */
		snprintf(m->ltsymbol, sizeof m->ltsymbol, "[%1u]", n);
	for (c = nexttiled(m->clients); c; c = nexttiled(c->next)) {
		animateclient(c, m->wx, m->wy, m->ww - 2 * c->bw, m->wh - 2 * c->bw, 7 * (animated && c == selmon->sel), 0);
	}

}

void
focusstack2(const Arg *arg)
{
	Client *nextVisibleClient = findVisibleClient(selmon->sel->next) ?: findVisibleClient(selmon->clients);

	if (nextVisibleClient) {
		if (nextVisibleClient->mon != selmon)
			selmon = nextVisibleClient->mon;
		detachstack(nextVisibleClient);
		attachstack(nextVisibleClient);
		selmon->sel = nextVisibleClient;
	}
}

void
overviewlayout(Monitor *m)
{
	int n;
	int gridwidth;
	unsigned int colwidth;
	unsigned int lineheight;
	int tmpx;
	int tmpy;
	Client *c;
	XWindowChanges wc;
	n = allclientcount();

	if (n == 0)
		return;

	gridwidth = 1;

	while ((gridwidth * gridwidth) < n) {
		gridwidth++;
	}

	tmpx = selmon->mx;
	tmpy = selmon->my + (selmon->showbar ? bh : 0);
	lineheight = selmon->wh / gridwidth;
	colwidth = selmon->ww / gridwidth;
	wc.stack_mode = Above;
	wc.sibling = m->barwin;

	for(c = m->clients; c; c = c->next) {
        if (HIDDEN(c))
            continue;
		if (c == selmon->overlay)
			continue;
		if (c->isfloating)
			savefloating(c);
		resize(c,tmpx, tmpy, c->w, c->h, 0);

		XConfigureWindow(dpy, c->win, CWSibling|CWStackMode, &wc);
		wc.sibling = c->win;
		if (tmpx + colwidth < selmon->mx + selmon->ww) {
			tmpx += colwidth;
		} else {
			tmpx = selmon->mx;
			tmpy += lineheight;
		}
	}
	XSync(dpy, False);
}

void
tcl(Monitor * m)
{
	int x, y, h, w, mw, sw, bdw;
	unsigned int i, n;
	Client * c;

	for (n = 0, c = nexttiled(m->clients); c;
			c = nexttiled(c->next), n++);

	if (n == 0)
		return;

	c = nexttiled(m->clients);

	mw = m->mfact * m->ww;
	sw = (m->ww - mw) / 2;
	bdw = (2 * c->bw);
	resize(c,
			n < 3 ? m->wx : m->wx + sw + m->gappx,
			m->wy + m->gappx,
			n == 1 ? m->ww - bdw : mw - bdw - 2*m->gappx,
			m->wh - bdw - 2*m->gappx,
			False);

	if (--n == 0)
		return;

	w = (m->ww - mw) / ((n > 1) + 1);
	c = nexttiled(c->next);

	if (n > 1)
	{
		x = m->wx + mw + sw;
		y = m->wy;
		h = m->wh / (n / 2);

		if (h < bh)
			h = m->wh;

		for (i = 0; c && i < n / 2; c = nexttiled(c->next), i++)
		{
			resize(c,
					x,
					y,
					w - bdw,
					(i + 1 == n / 2) ? m->wy + m->wh - y - bdw : h - bdw,
					False);

			if (h != m->wh)
				y = c->y + HEIGHT(c);
		}
	}

	x = (n + 1 / 2) == 1 ? mw + m->wx : m->wx;
	y = m->wy;
	h = m->wh / ((n + 1) / 2);

	if (h < bh)
		h = m->wh;

	int rw = w - bdw;

	for (i = 0; c; c = nexttiled(c->next), i++)
	{
		int rh = (i + 1 == (n + 1) / 2) ? m->wy + m->wh - y - bdw : h - bdw;
		resize(c, x, y, rw,	rh, 0);

		if (h != m->wh)
			y = c->y + HEIGHT(c);
	}
}

void
tile(Monitor *m)
{
	unsigned int i, n, h, mw, my, ty, framecount;
	Client *c;

	if (animated && clientcount() > 5)
		framecount = 4;
	else
		framecount = 7;

	for (n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++);
	if (n == 0)
		return;

	if (n > m->nmaster)
		mw = m->nmaster ? m->ww * m->mfact - m->gappx : 0;
	else {
		mw = m->ww - m->gappx;
		if (n > 1 && n < m->nmaster) {
			m->nmaster = n;
			tile(m);
			return;
		}
	}
	for (i = 0, my = ty = m->gappx, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++)
		if (i < m->nmaster) {
			// client is in the master
			h = (m->wh - my) / (MIN(n, m->nmaster) - i) - m->gappx;

            if (n == 2) {
                animateclient(c, m->wx + m->gappx, m->wy + my, mw - (2*c->bw) - m->gappx, h - (2*c->bw), 0, 0);
            } else {
			animateclient(c, m->wx + m->gappx, m->wy + my, mw - (2*c->bw) - m->gappx, h - (2*c->bw), framecount, 0);
			if (m->nmaster == 1 && n > 1) {
				mw = c->w + c->bw * 2 + m->gappx;
			}
            }
			if (my + HEIGHT(c) < m->wh)
				my += HEIGHT(c) + m->gappx;
		} else {
			// client is in the stack
			h = (m->wh - ty) / (n - i) - m->gappx;
            animateclient(c, m->wx + mw + m->gappx, m->wy + ty, m->ww - mw - (2*c->bw) - 2*m->gappx, h - (2*c->bw), framecount, 0);
			if (ty + HEIGHT(c) < m->wh)
				ty += HEIGHT(c) + m->gappx;
		}
}
