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

#ifndef PULSEVIEW_PV_POPUPS_DEVICEOPTIONS_HPP
#define PULSEVIEW_PV_POPUPS_DEVICEOPTIONS_HPP

#include <QGroupBox>
#include <QVBoxLayout>

#include <pv/binding/device.hpp>
#include <pv/widgets/popup.hpp>

namespace sigrok {
	class Device;
}

namespace pv {
namespace popups {

class DeviceOptions : public pv::widgets::Popup
{
	Q_OBJECT

public:
	DeviceOptions(std::shared_ptr<sigrok::Device> device,
		QWidget *parent);

	pv::binding::Device& binding();

private:
	std::shared_ptr<sigrok::Device> device_;

	QVBoxLayout layout_;

	pv::binding::Device binding_;
};

} // namespace popups
} // namespace pv

#endif // PULSEVIEW_PV_POPUPS_DEVICEOPTIONS_HPP
