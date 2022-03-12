#include "cairomm/enums.h"
#include "gtkmm/enums.h"
#include <gdkmm/window.h>
#include <gtkmm.h>
#include <gtkmm/application.h>
#include <gtkmm/button.h>
#include <gtkmm/window.h>
#include <gtkmm/image.h>
#include <gtkmm/box.h>
#include <gtk-layer-shell/gtk-layer-shell.h>
#include <iostream>
#include <cmath>
#include <vector>

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
		m_image.set_focus_on_click(false);
		m_image.set_visible(false);

		set_opacity(0.4);
		set_visible(false);
		set_can_focus(false);
		set_resizable(false);
		set_app_paintable(true);
		set_type_hint(Gdk::WINDOW_TYPE_HINT_NOTIFICATION);
		set_title("wlosd");

		m_box.set_visible(false);
		m_box.set_can_focus(false);
		m_box.set_orientation(Gtk::ORIENTATION_VERTICAL);
		m_box.set_spacing(0);

		add(m_box);

		drawslot = sigc::mem_fun(*this, &OsdWindow::my_on_draw);

		signal_draw().connect(drawslot, false);

		m_box.add(m_image);

		tslot = sigc::mem_fun(*this, &OsdWindow::timeout);
		sslot = sigc::mem_fun(*this, &OsdWindow::showstatus);
		Glib::signal_timeout().connect(sslot, 5000);

		background.set_rgba(160, 160, 160, 0.8);

		set_size_request(_width, _height);
	}

    private:
	bool timeout()
	{
		m_box.set_visible(false);

		return false;
	}

	bool showstatus()
	{
		show_all();
		m_box.show();
		m_image.show();
		m_box.pack_start(m_image, true, true, 0);
		// gtk_layer_set_monitor(gobj(), this->get_display()->get_monitor(1)->gobj());

		Gdk::Rectangle wa;
		get_display()->get_monitor(0)->get_workarea(wa);

		double margin = wa.get_height() - _height * 0.75;

		std::cout << margin << std::endl;

		gtk_layer_set_margin(gobj(), GTK_LAYER_SHELL_EDGE_TOP, margin);

		queue_draw();
		Glib::signal_timeout().connect(tslot, 1500);
		return true;
	}

	void draw_rect(const Cairo::RefPtr<Cairo::Context> &ctx, double x, double y, double w,
		       double h)
	{
		ctx->line_to(x, y);
		ctx->line_to(x + w, y);
		ctx->line_to(x + w, y + h);
		ctx->line_to(x, y + h);

		ctx->close_path();
		ctx->fill();
	}

	void draw_round_rect(const Cairo::RefPtr<Cairo::Context> &ctx, double x, double y, double w,
			     double h, double r)
	{
		ctx->move_to(x + r, y);
		ctx->line_to(x + w - r, y);
		ctx->arc(x + w - r, y + r, r, -M_PI / 2.0, 0);

		ctx->line_to(x + w, y + h - r);

		ctx->arc(x + w - r, y + h - r, r, 0, M_PI / 2.0);

		ctx->line_to(x + r, y + h);

		ctx->arc(x + r, y + h - r, r, M_PI / 2.0, M_PI);

		ctx->line_to(x, y + r);

		ctx->arc(x + r, y + r, r, M_PI, 3 * M_PI / 2);

		ctx->close_path();
		ctx->fill();
	}

	bool my_on_draw(const Cairo::RefPtr<Cairo::Context> &ctx)
	{
		if (m_box.is_visible()) {
			ctx->set_operator(Cairo::OPERATOR_SOURCE);
			ctx->paint();

			ctx->set_operator(Cairo::OPERATOR_CLEAR);

			draw_rect(ctx, 0, 0, _width, _height);

			ctx->set_operator(Cairo::OPERATOR_SOURCE);
			Gdk::Cairo::set_source_rgba(ctx, background);
			draw_round_rect(ctx, 0, 0, _width, _height, border_radius);

			ctx->set_operator(Cairo::OPERATOR_OVER);
		}

		return false;
	}

    protected:
	Gtk::Box m_box;
	Gtk::Image m_image;
	sigc::slot<bool()> tslot;
	sigc::slot<bool()> sslot;
	sigc::slot<bool(const Cairo::RefPtr<Cairo::Context> &cr)> drawslot;

	Gdk::RGBA background;

	int border_radius = 32;

	int _width = 248;
	int _height = 232;
};

int main(int argc, char *argv[])
{
	auto app = Gtk::Application::create("org.hello", Gio::APPLICATION_FLAGS_NONE);
	OsdWindow d;

	app->run(d);

	return 0;
}
