/*
 * This file is part of the PulseView project.
 *
 * Copyright (C) 2012 Joel Holdsworth <joel@airwebreathe.org.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <extdef.h>

#include <assert.h>

#include <QAction>
#include <QDebug>
#include <QHelpEvent>
#include <QMenu>
#include <QToolTip>

#include "mainbar.hpp"

#include <pv/devicemanager.hpp>
#include <pv/mainwindow.hpp>
#include <pv/popups/deviceoptions.hpp>
#include <pv/popups/channels.hpp>
#include <pv/util.hpp>
#include <pv/view/view.hpp>

#ifdef ENABLE_DECODE
#include <pv/widgets/decodermenu.hpp>
#endif

#include <libsigrok/libsigrok.hpp>

using std::map;
using std::vector;
using std::max;
using std::min;
using std::shared_ptr;
using std::string;

using sigrok::Capability;
using sigrok::ConfigKey;
using sigrok::Device;
using sigrok::Error;

namespace pv {
namespace toolbars {

const uint64_t MainBar::MinSampleCount = 100ULL;
const uint64_t MainBar::MaxSampleCount = 1000000000000ULL;
const uint64_t MainBar::DefaultSampleCount = 1000000;

MainBar::MainBar(Session &session, MainWindow &main_window) :
	QToolBar("Sampling Bar", &main_window),
	session_(session),
	main_window_(main_window),
	device_selector_(this),
	updating_device_selector_(false),
	configure_button_(this),
	configure_button_action_(NULL),
	channels_button_(this),
	sample_count_(" samples", this),
	sample_rate_("Hz", this),
	updating_sample_rate_(false),
	updating_sample_count_(false),
	sample_count_supported_(false),
	icon_red_(":/icons/status-red.svg"),
	icon_green_(":/icons/status-green.svg"),
	icon_grey_(":/icons/status-grey.svg"),
	run_stop_button_(this),
	menu_button_(this)
{
	setObjectName(QString::fromUtf8("MainBar"));

	setMovable(false);
	setFloatable(false);

	// Setup the menu bar
	QMenu *const menu = new QMenu(this);

	// File Menu
	QMenu *const menu_file = new QMenu;
	menu_file->setTitle(tr("&File"));

	QAction *const action_connect = new QAction(this);
	action_connect->setText(tr("&Connect to Device..."));
	action_connect->setObjectName(QString::fromUtf8("actionConnect"));
	menu_file->addAction(action_connect);

	menu->addAction(menu_file->menuAction());

	// View Menu
	QMenu *menu_view = new QMenu;
	menu_view->setTitle(tr("&View"));

	QAction *action_view_show_cursors = new QAction(this);
	action_view_show_cursors->setCheckable(true);
	action_view_show_cursors->setChecked(
		main_window.view()->cursors_shown());
	action_view_show_cursors->setShortcut(QKeySequence(Qt::Key_C));
	action_view_show_cursors->setObjectName(
		QString::fromUtf8("actionViewShowCursors"));
	action_view_show_cursors->setText(tr("Show &Cursors"));
	menu_view->addAction(action_view_show_cursors);

	menu->addAction(menu_view->menuAction());

	// Decoders Menu
#ifdef ENABLE_DECODE
	QMenu *const menu_decoders = new QMenu;
	menu_decoders->setTitle(tr("&Decoders"));

	pv::widgets::DecoderMenu *const menu_decoders_add =
		new pv::widgets::DecoderMenu(menu_decoders, true);
	menu_decoders_add->setTitle(tr("&Add"));
	connect(menu_decoders_add, SIGNAL(decoder_selected(srd_decoder*)),
		this, SIGNAL(decoder_selected(srd_decoder*)));

	menu_decoders->addMenu(menu_decoders_add);

	menu->addAction(menu_decoders->menuAction());
#endif

	// Help Menu
	QMenu *const menu_help = new QMenu;
	menu_help->setTitle(tr("&Help"));

	QAction *const action_about = new QAction(this);
	action_about->setObjectName(QString::fromUtf8("actionAbout"));
	action_about->setText(tr("&About..."));
	menu_help->addAction(action_about);

	menu->addAction(menu_help->menuAction());

	// Quit item
	menu->addSeparator();

	QAction *action_quit = new QAction(this);
	action_quit->setText(tr("&Quit"));
	action_quit->setIcon(QIcon::fromTheme("application-exit",
		QIcon(":/icons/application-exit.png")));
	action_quit->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));
	action_quit->setObjectName(QString::fromUtf8("actionQuit"));
	menu->addAction(action_quit);

	menu_button_.setMenu(menu);
	menu_button_.setPopupMode(QToolButton::InstantPopup);
	menu_button_.setIcon(QIcon::fromTheme("menu",
		QIcon(":/icons/menu.svg")));

	// Setup the toolbar
	QAction *const action_open = new QAction(this);
	action_open->setText(tr("&Open..."));
	action_open->setIcon(QIcon::fromTheme("document-open",
		QIcon(":/icons/document-open.png")));
	action_open->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_O));
	action_open->setObjectName(QString::fromUtf8("actionOpen"));

	QAction *const action_save_as = new QAction(this);
	action_save_as->setText(tr("&Save As..."));
	action_save_as->setIcon(QIcon::fromTheme("document-save-as",
		QIcon(":/icons/document-save-as.png")));
	action_save_as->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
	action_save_as->setObjectName(QString::fromUtf8("actionSaveAs"));

	QAction *const action_view_zoom_in = new QAction(this);
	action_view_zoom_in->setText(tr("Zoom &In"));
	action_view_zoom_in->setIcon(QIcon::fromTheme("zoom-in",
		QIcon(":/icons/zoom-in.png")));
	// simply using Qt::Key_Plus shows no + in the menu
	action_view_zoom_in->setShortcut(QKeySequence::ZoomIn);
	action_view_zoom_in->setObjectName(
		QString::fromUtf8("actionViewZoomIn"));

	QAction *const action_view_zoom_out = new QAction(this);
	action_view_zoom_out->setText(tr("Zoom &Out"));
	action_view_zoom_out->setIcon(QIcon::fromTheme("zoom-out",
		QIcon(":/icons/zoom-out.png")));
	action_view_zoom_out->setShortcut(QKeySequence::ZoomOut);
	action_view_zoom_out->setObjectName(
		QString::fromUtf8("actionViewZoomOut"));

	QAction *const action_view_zoom_fit = new QAction(this);
	action_view_zoom_fit->setText(tr("Zoom to &Fit"));
	action_view_zoom_fit->setIcon(QIcon::fromTheme("zoom-fit",
		QIcon(":/icons/zoom-fit.png")));
	action_view_zoom_fit->setShortcut(QKeySequence(Qt::Key_F));
	action_view_zoom_fit->setObjectName(
		QString::fromUtf8("actionViewZoomFit"));

	QAction *const action_view_zoom_one_to_one = new QAction(this);
	action_view_zoom_one_to_one->setText(tr("Zoom to &One-to-One"));
	action_view_zoom_one_to_one->setIcon(QIcon::fromTheme("zoom-original",
		QIcon(":/icons/zoom-original.png")));
	action_view_zoom_one_to_one->setShortcut(QKeySequence(Qt::Key_O));
	action_view_zoom_one_to_one->setObjectName(
		QString::fromUtf8("actionViewZoomOneToOne"));

	addAction(action_open);
	addAction(action_save_as);
	addSeparator();
	addAction(action_view_zoom_in);
	addAction(action_view_zoom_out);
	addAction(action_view_zoom_fit);
	addAction(action_view_zoom_one_to_one);
	addSeparator();

	connect(&run_stop_button_, SIGNAL(clicked()),
		this, SLOT(on_run_stop()));
	connect(&device_selector_, SIGNAL(currentIndexChanged (int)),
		this, SLOT(on_device_selected()));
	connect(&sample_count_, SIGNAL(value_changed()),
		this, SLOT(on_sample_count_changed()));
	connect(&sample_rate_, SIGNAL(value_changed()),
		this, SLOT(on_sample_rate_changed()));

	sample_count_.show_min_max_step(0, UINT64_MAX, 1);

	set_capture_state(pv::Session::Stopped);

	configure_button_.setIcon(QIcon::fromTheme("configure",
		QIcon(":/icons/configure.png")));

	channels_button_.setIcon(QIcon::fromTheme("channels",
		QIcon(":/icons/channels.svg")));

	run_stop_button_.setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

	addWidget(&device_selector_);
	configure_button_action_ = addWidget(&configure_button_);
	addWidget(&channels_button_);
	addWidget(&sample_count_);
	addWidget(&sample_rate_);
	addWidget(&run_stop_button_);

	QWidget *const spacer = new QWidget();
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	addWidget(spacer);

	addWidget(&menu_button_);

	sample_count_.installEventFilter(this);
	sample_rate_.installEventFilter(this);
}

void MainBar::set_device_list(
	const std::list< std::shared_ptr<sigrok::Device> > &devices,
	shared_ptr<Device> selected)
{
	int selected_index = -1;

	assert(selected);

	updating_device_selector_ = true;

	device_selector_.clear();

	for (auto device : devices) {
		assert(device);

		string display_name =
			session_.device_manager().get_display_name(device);

		if (selected == device)
			selected_index = device_selector_.count();

		device_selector_.addItem(display_name.c_str(),
			qVariantFromValue(device));
	}

	// The selected device should have been in the list
	assert(selected_index != -1);
	device_selector_.setCurrentIndex(selected_index);

	update_device_config_widgets();

	updating_device_selector_ = false;
}

shared_ptr<Device> MainBar::get_selected_device() const
{
	const int index = device_selector_.currentIndex();
	if (index < 0)
		return shared_ptr<Device>();

	return device_selector_.itemData(index).value<shared_ptr<Device>>();
}

void MainBar::set_capture_state(pv::Session::capture_state state)
{
	const QIcon *icons[] = {&icon_grey_, &icon_red_, &icon_green_};
	run_stop_button_.setIcon(*icons[state]);
	run_stop_button_.setText((state == pv::Session::Stopped) ?
		tr("Run") : tr("Stop"));
	run_stop_button_.setShortcut(QKeySequence(Qt::Key_Space));
}

void MainBar::update_sample_rate_selector()
{
	Glib::VariantContainerBase gvar_dict;
	GVariant *gvar_list;
	const uint64_t *elements = NULL;
	gsize num_elements;

	if (updating_sample_rate_)
		return;

	const shared_ptr<Device> device = get_selected_device();
	if (!device)
		return;

	assert(!updating_sample_rate_);
	updating_sample_rate_ = true;

	const auto keys = device->config_keys(ConfigKey::DEVICE_OPTIONS);
	const auto iter = keys.find(ConfigKey::SAMPLERATE);
	if (iter != keys.end() &&
		(*iter).second.find(sigrok::LIST) != (*iter).second.end()) {
		gvar_dict = device->config_list(ConfigKey::SAMPLERATE);
	} else {
		sample_rate_.show_none();
		updating_sample_rate_ = false;
		return;
	}

	if ((gvar_list = g_variant_lookup_value(gvar_dict.gobj(),
			"samplerate-steps", G_VARIANT_TYPE("at"))))
	{
		elements = (const uint64_t *)g_variant_get_fixed_array(
				gvar_list, &num_elements, sizeof(uint64_t));

		const uint64_t min = elements[0];
		const uint64_t max = elements[1];
		const uint64_t step = elements[2];

		g_variant_unref(gvar_list);

		assert(min > 0);
		assert(max > 0);
		assert(max > min);
		assert(step > 0);

		if (step == 1)
			sample_rate_.show_125_list(min, max);
		else
		{
			// When the step is not 1, we cam't make a 1-2-5-10
			// list of sample rates, because we may not be able to
			// make round numbers. Therefore in this case, show a
			// spin box.
			sample_rate_.show_min_max_step(min, max, step);
		}
	}
	else if ((gvar_list = g_variant_lookup_value(gvar_dict.gobj(),
			"samplerates", G_VARIANT_TYPE("at"))))
	{
		elements = (const uint64_t *)g_variant_get_fixed_array(
				gvar_list, &num_elements, sizeof(uint64_t));
		sample_rate_.show_list(elements, num_elements);
		g_variant_unref(gvar_list);
	}
	updating_sample_rate_ = false;

	update_sample_rate_selector_value();
}

void MainBar::update_sample_rate_selector_value()
{
	if (updating_sample_rate_)
		return;

	const shared_ptr<Device> device = get_selected_device();
	if (!device)
		return;

	try {
		auto gvar = device->config_get(ConfigKey::SAMPLERATE);
		uint64_t samplerate =
			Glib::VariantBase::cast_dynamic<Glib::Variant<guint64>>(gvar).get();
		assert(!updating_sample_rate_);
		updating_sample_rate_ = true;
		sample_rate_.set_value(samplerate);
		updating_sample_rate_ = false;
	} catch (Error error) {
		qDebug() << "WARNING: Failed to get value of sample rate";
		return;
	}
}

void MainBar::update_sample_count_selector()
{
	if (updating_sample_count_)
		return;

	const shared_ptr<Device> device = get_selected_device();
	if (!device)
		return;

	assert(!updating_sample_count_);
	updating_sample_count_ = true;

	if (!sample_count_supported_)
	{
		sample_count_.show_none();
		updating_sample_count_ = false;
		return;
	}

	uint64_t sample_count = sample_count_.value();
	uint64_t min_sample_count = 0;
	uint64_t max_sample_count = MaxSampleCount;

	if (sample_count == 0)
		sample_count = DefaultSampleCount;

	const auto keys = device->config_keys(ConfigKey::DEVICE_OPTIONS);
	const auto iter = keys.find(ConfigKey::LIMIT_SAMPLES);
	if (iter != keys.end() &&
		(*iter).second.find(sigrok::LIST) != (*iter).second.end()) {
		auto gvar = device->config_list(ConfigKey::LIMIT_SAMPLES);
		g_variant_get(gvar.gobj(), "(tt)",
			&min_sample_count, &max_sample_count);
	}

	min_sample_count = min(max(min_sample_count, MinSampleCount),
		max_sample_count);

	sample_count_.show_125_list(
		min_sample_count, max_sample_count);

	try {
		auto gvar = device->config_get(ConfigKey::LIMIT_SAMPLES);
		sample_count = g_variant_get_uint64(gvar.gobj());
		if (sample_count == 0)
			sample_count = DefaultSampleCount;
		sample_count = min(max(sample_count, MinSampleCount),
			max_sample_count);
	} catch (Error error) {}

	sample_count_.set_value(sample_count);

	updating_sample_count_ = false;
}

void MainBar::update_device_config_widgets()
{
	using namespace pv::popups;

	const shared_ptr<Device> device = get_selected_device();
	if (!device)
		return;

	// Update the configure popup
	DeviceOptions *const opts = new DeviceOptions(device, this);
	configure_button_action_->setVisible(
		!opts->binding().properties().empty());
	configure_button_.set_popup(opts);

	// Update the channels popup
	Channels *const channels = new Channels(session_, this);
	channels_button_.set_popup(channels);

	// Update supported options.
	sample_count_supported_ = false;

	try {
		for (auto entry : device->config_keys(ConfigKey::DEVICE_OPTIONS))
		{
			auto key = entry.first;
			auto capabilities = entry.second;
			switch (key->id()) {
			case SR_CONF_LIMIT_SAMPLES:
				if (capabilities.count(Capability::SET))
					sample_count_supported_ = true;
				break;
			case SR_CONF_LIMIT_FRAMES:
				if (capabilities.count(Capability::SET))
				{
					device->config_set(ConfigKey::LIMIT_FRAMES,
						Glib::Variant<guint64>::create(1));
					on_config_changed();
				}
				break;
			default:
				break;
			}
		}
	} catch (Error error) {}

	// Add notification of reconfigure events
	disconnect(this, SLOT(on_config_changed()));
	connect(&opts->binding(), SIGNAL(config_changed()),
		this, SLOT(on_config_changed()));

	// Update sweep timing widgets.
	update_sample_count_selector();
	update_sample_rate_selector();
}

void MainBar::commit_sample_count()
{
	uint64_t sample_count = 0;

	if (updating_sample_count_)
		return;

	const shared_ptr<Device> device = get_selected_device();
	if (!device)
		return;

	sample_count = sample_count_.value();

	// Set the sample count
	assert(!updating_sample_count_);
	updating_sample_count_ = true;
	if (sample_count_supported_)
	{
		try {
			device->config_set(ConfigKey::LIMIT_SAMPLES,
				Glib::Variant<guint64>::create(sample_count));
			on_config_changed();
		} catch (Error error) {
			qDebug() << "Failed to configure sample count.";
			return;
		}
	}
	updating_sample_count_ = false;
}

void MainBar::commit_sample_rate()
{
	uint64_t sample_rate = 0;

	if (updating_sample_rate_)
		return;

	const shared_ptr<Device> device = get_selected_device();
	if (!device)
		return;

	sample_rate = sample_rate_.value();
	if (sample_rate == 0)
		return;

	// Set the samplerate
	assert(!updating_sample_rate_);
	updating_sample_rate_ = true;
	try {
		device->config_set(ConfigKey::SAMPLERATE,
			Glib::Variant<guint64>::create(sample_rate));
		on_config_changed();
	} catch (Error error) {
		qDebug() << "Failed to configure samplerate.";
		return;
	}
	updating_sample_rate_ = false;
}

void MainBar::on_device_selected()
{
	if (updating_device_selector_)
		return;

	shared_ptr<Device> device = get_selected_device();
	if (!device)
		return;

	main_window_.select_device(device);

	update_device_config_widgets();
}

void MainBar::on_sample_count_changed()
{
	commit_sample_count();
}

void MainBar::on_sample_rate_changed()
{
	commit_sample_rate();
}

void MainBar::on_run_stop()
{
	commit_sample_count();
	commit_sample_rate();	
	main_window_.run_stop();
}

void MainBar::on_config_changed()
{
	commit_sample_count();
	update_sample_count_selector();	
	commit_sample_rate();	
	update_sample_rate_selector();
}

bool MainBar::eventFilter(QObject *watched, QEvent *event)
{
	if ((watched == &sample_count_ || watched == &sample_rate_) &&
		(event->type() == QEvent::ToolTip)) {
		double sec = (double)sample_count_.value() / sample_rate_.value();
		QHelpEvent *help_event = static_cast<QHelpEvent*>(event);

		QString str = tr("Total sampling time: %1").arg(pv::util::format_second(sec));
		QToolTip::showText(help_event->globalPos(), str);

		return true;
	}

	return false;
}

} // namespace toolbars
} // namespace pv
