
#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gdk/gdkx.h>
#include <gtk/gtk.h>

#include <glib/gi18n-lib.h>

#include "xapp-display.h"

struct _XAppDisplayPrivate
{
  int num_outputs;
  GtkWidget **windows;
};

G_DEFINE_TYPE (XAppDisplay, xapp_display, G_TYPE_OBJECT);

GtkWidget * create_blanking_window (GdkScreen *screen, int monitor);

static void
xapp_display_init (XAppDisplay *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, XAPP_TYPE_DISPLAY, XAppDisplayPrivate);
}

static void
xapp_display_finalize (GObject *object)
{
  XAppDisplay *self = XAPP_DISPLAY (object);

  if (self->priv->windows != NULL) {
    xapp_display_unblank_monitors (XAPP_DISPLAY(self));
    g_free (self->priv->windows);
  }

  G_OBJECT_CLASS (xapp_display_parent_class)->finalize (object);
}

static void
xapp_display_class_init (XAppDisplayClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = xapp_display_finalize;

  g_type_class_add_private (gobject_class, sizeof (XAppDisplayPrivate));
}

XAppDisplay *
xapp_display_new (void)
{
  return g_object_new (XAPP_TYPE_DISPLAY, NULL);
}

GtkWidget *
create_blanking_window (GdkScreen *screen, int monitor)
{
  GdkRectangle fullscreen;
  gdk_screen_get_monitor_geometry(screen, monitor, &fullscreen);
  GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (window), TRUE);
  gtk_window_set_skip_pager_hint (GTK_WINDOW (window), TRUE);
  gtk_window_resize (GTK_WINDOW (window), fullscreen.width, fullscreen.height);
  gtk_window_move (GTK_WINDOW (window), fullscreen.x, fullscreen.y);
  gtk_widget_set_visible (window, TRUE);
  return window;
}

void
xapp_display_blank_other_monitors (XAppDisplay *self, GtkWindow *window)
{
  int i;

  g_return_if_fail (XAPP_IS_DISPLAY (self));

  if (self->priv->windows != NULL)
    return;

  GdkScreen *screen = gtk_window_get_screen (window);
  int active_monitor = gdk_screen_get_monitor_at_window (screen, gtk_widget_get_window(window));
  self->priv->num_outputs = gdk_screen_get_n_monitors (screen);
  self->priv->windows = g_new (GtkWidget *, self->priv->num_outputs);

  for (i = 0; i < self->priv->num_outputs; i++) {
  	if (i != active_monitor) { 
    	self->priv->windows[i] = create_blanking_window (screen, i);
    }
    else {
    	// initialize at NULL so it gets properly skipped when windows get destroyed
    	self->priv->windows[i] = NULL;
    }
  }
}

void
xapp_display_unblank_monitors (XAppDisplay *self)
{
  int i;
  g_return_if_fail (XAPP_IS_DISPLAY (self));
  if (self->priv->windows == NULL)
    return;

  for (i = 0; i < self->priv->num_outputs; i++) {
    if (self->priv->windows[i] != NULL) {
      gtk_widget_destroy (self->priv->windows[i]);
      self->priv->windows[i] = NULL;
    }
  }
  g_free (self->priv->windows);
  self->priv->windows = NULL;
}
