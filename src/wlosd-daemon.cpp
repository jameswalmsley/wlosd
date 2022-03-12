#include <gtkmm/application.h>
#include <gtkmm/button.h>
#include <gtkmm/window.h>
#include <gtkmm/image.h>

class Daemon : public Gtk::Window {
    public:
	Daemon() : m_image("images/volume_muted.png")
	{
		m_image.set_from_resource("/wlosd/images/volume_muted.png");
		add(m_image);
		show_all();
	}

    protected:
	Gtk::Image m_image;
};

int main(int argc, char *argv[])
{
	auto app = Gtk::Application::create("org.hello", Gio::APPLICATION_FLAGS_NONE);
	Daemon d;

	app->run(d);

	return 0;
}
