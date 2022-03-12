#include "gdkmm/window.h"
#include <gtkmm/application.h>
#include <gtkmm/button.h>
#include <gtkmm/window.h>
#include <gtkmm/image.h>
#include <gtkmm/box.h>
#include <gtk-layer-shell/gtk-layer-shell.h>

class OsdWindow : public Gtk::Window {
    public:
	OsdWindow() : m_image("images/volume_muted.png")
	{
		auto screen = get_screen();
		auto visual = screen->get_rgba_visual();

		gtk_layer_init_for_window(gobj());
		gtk_layer_set_layer(gobj(), GTK_LAYER_SHELL_LAYER_OVERLAY);
		gtk_layer_set_anchor(gobj(), GTK_LAYER_SHELL_EDGE_TOP, true);
		gtk_layer_set_exclusive_zone(gobj(), -1);
		gtk_layer_set_keyboard_interactivity(gobj(), false);

		m_image.set_from_resource("/wlosd/images/volume_muted.png");

		set_opacity(0.4);
		set_visible(false);
		set_can_focus(false);
		set_resizable(false);
		set_app_paintable(true);
		set_type_hint(Gdk::WINDOW_TYPE_HINT_NOTIFICATION);
		set_title("wlosd");

		m_box.set_visible(true);

		add(m_box);

		m_box.add(m_image);

		show_all();
	}

	void draw(const ::Cairo::RefPtr< ::Cairo::Context> &cr) {
	    printf("Hello");
	}
	

    protected:
	Gtk::Box m_box;
	Gtk::Image m_image;
};

int main(int argc, char *argv[])
{
	auto app = Gtk::Application::create("org.hello", Gio::APPLICATION_FLAGS_NONE);
	OsdWindow d;

	app->run(d);

	return 0;
}
