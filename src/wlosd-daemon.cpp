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
		m_image.set_from_resource("/wlosd/images/volume_muted.png");
		m_image.set_focus_on_click(false);
		m_image.set_visible(false);

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

		background.set_rgba(160, 160, 160, 0.8);
		block_bg_color.set_rgba(106, 106, 106, 0.8);
		block_fg_color.set_rgba(0, 0, 0, 0.8);

		set_size_request(_width, _height);

		m_box.set_spacing(0);
		m_image.set_vexpand();

		set_opacity(0.9);

		auto visual = get_screen()->get_rgba_visual();
		//gtk_widget_set_visual(this->get_default_widget()->gobj(), visual->gobj());
	}

    private:
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

			int block_width = (_width -2 * _padding - ((_block_count - 1) * _block_spacing)) / _block_count;

			int blocks_x = _padding;
			int blocks_y = _height - _padding - _block_height;

			Gdk::Cairo::set_source_rgba(ctx, block_bg_color);

			for(int i = 0; i < _block_count; i++) {
			    draw_rect(ctx, blocks_x + (block_width + _block_spacing) * i,
				    blocks_y,
				    block_width,
				    _block_height);
			}

			Gdk::Cairo::set_source_rgba(ctx, block_fg_color);

			for(int i = 0; i < _block_count * _progress; i++) {
			    draw_rect(ctx, blocks_x + (block_width + _block_spacing) * i,
				    blocks_y,
				    block_width,
				    _block_height);
			}
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
	Gdk::RGBA block_bg_color;
	Gdk::RGBA block_fg_color;

	int border_radius = 16;

	double _progress = 0.5;

	int _width = 248;
	int _height = 232;
	int _padding = 24;
	int _block_count = 20;
	int _block_spacing = 2;
	int _block_height = 10;
};

class OsdService {
    public:
	OsdService()
	{
	    sslot = sigc::mem_fun(*this, &OsdService::show);
	    Glib::signal_timeout().connect(sslot, 2000);
	}

	bool show() {
	    auto display = Gdk::Display::get_default();
	    auto monitors = display->get_n_monitors();

	    if(_windows.size() < monitors) {
		for(int i=0; i < monitors-_windows.size(); i++) {
		    _windows.push_back(nullptr);
		}
	    }

	    for(int i = 0; i < monitors; i++) {
		auto window = _windows[i];
		if(!window) {
		    window = create_window();
		    _windows[i] = window;
		}

		show_window(window, display->get_monitor(i));
	    }

	    return false;
	}

    private:

	void show_window(OsdWindow* w, Glib::RefPtr<Gdk::Monitor> m) {
	    gtk_layer_set_monitor(w->gobj(), m->gobj());

	    Gdk::Rectangle wa;
	    m->get_workarea(wa);

	    double margin = (wa.get_height() - 232) * 0.75;
	    gtk_layer_set_margin(w->gobj(), GTK_LAYER_SHELL_EDGE_TOP, margin);

	     w->show_all();
	}

	OsdWindow* create_window() {
	    auto window = new OsdWindow();
	    gtk_layer_init_for_window(window->gobj());
	    gtk_layer_set_layer(window->gobj(), GTK_LAYER_SHELL_LAYER_OVERLAY);
	    gtk_layer_set_anchor(window->gobj(), GTK_LAYER_SHELL_EDGE_TOP, true);
	    gtk_layer_set_exclusive_zone(window->gobj(), 0);
	    gtk_layer_set_keyboard_interactivity(window->gobj(), false);

	    return window;
	}

	sigc::slot<bool()> sslot;
	std::vector<OsdWindow*> _windows;

};

int main(int argc, char *argv[])
{
	auto app = Gtk::Application::create("org.jameswalmsley.wlosd", Gio::APPLICATION_FLAGS_NONE);

	OsdService s;

	app->hold();
	app->run();

	return 0;
}
