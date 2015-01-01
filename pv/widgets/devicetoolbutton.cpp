/*
 * This file is part of the PulseView project.
 *
 * Copyright (C) 2015 Joel Holdsworth <joel@airwebreathe.org.uk>
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

#include <cassert>

#include <libsigrok/libsigrok.hpp>

#include <pv/devicemanager.hpp>

#include "devicetoolbutton.hpp"

using std::list;
using std::shared_ptr;
using std::string;
using std::weak_ptr;
using std::vector;

using sigrok::Device;

namespace pv {
namespace widgets {

DeviceToolButton::DeviceToolButton(QWidget *parent,
	DeviceManager &device_manager) :
	QToolButton(parent),
	device_manager_(device_manager),
	menu_(this),
	mapper_(this),
	connect_action_(this),
	devices_()
{
	connect_action_.setText(tr("&Connect to Device..."));
	connect_action_.setObjectName(QString("actionConnect"));

	setPopupMode(QToolButton::MenuButtonPopup);
	setMenu(&menu_);
	setDefaultAction(&connect_action_);
	setMinimumWidth(QFontMetrics(font()).averageCharWidth() * 24);

	connect(&mapper_, SIGNAL(mapped(QObject*)),
		this, SLOT(on_action(QObject*)));
}

QAction* DeviceToolButton::connect_action()
{
	return &connect_action_;
}

shared_ptr<Device> DeviceToolButton::selected_device()
{
	return selected_device_;
}

void DeviceToolButton::set_device_list(
	const list< shared_ptr<Device> > &devices, shared_ptr<Device> selected)
{
	selected_device_ = selected;
	setText(QString::fromStdString(
		device_manager_.get_display_name(selected)));
	devices_ = vector< weak_ptr<Device> >(devices.begin(), devices.end());
	update_device_list();
}

void DeviceToolButton::update_device_list()
{
	menu_.clear();
	menu_.addAction(&connect_action_);
	menu_.setDefaultAction(&connect_action_);
	menu_.addSeparator();

	for (weak_ptr<Device> dev_weak_ptr : devices_) {
		shared_ptr<Device> dev(dev_weak_ptr);
		if (!dev)
			continue;

		QAction *const a = new QAction(QString::fromStdString(
			device_manager_.get_display_name(dev)), this);
		a->setCheckable(true);
		a->setChecked(selected_device_ == dev);
		a->setData(qVariantFromValue((void*)dev.get()));
		mapper_.setMapping(a, a);

		connect(a, SIGNAL(triggered()), &mapper_, SLOT(map()));

		menu_.addAction(a);
	}
}

void DeviceToolButton::on_action(QObject *action)
{
	assert(action);

	Device *const dev = (Device*)((QAction*)action)->data().value<void*>();
	for (weak_ptr<Device> dev_weak_ptr : devices_) {
		shared_ptr<Device> dev_ptr(dev_weak_ptr);
		if (dev_ptr.get() == dev) {
			selected_device_ = shared_ptr<Device>(dev_ptr);
			break;
		}
	}

	update_device_list();
	setText(QString::fromStdString(
		device_manager_.get_display_name(selected_device_)));

	device_selected();
}

} // widgets
} // pv
