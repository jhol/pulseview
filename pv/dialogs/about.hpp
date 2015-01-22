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

#ifndef PULSEVIEW_PV_ABOUT_HPP
#define PULSEVIEW_PV_ABOUT_HPP

#include <QDialog>

#include <memory>

class QTextDocument;

namespace sigrok {
	class Context;
}

namespace Ui {
class About;
}

namespace pv {
namespace dialogs {

class About : public QDialog
{
	Q_OBJECT

public:
	explicit About(std::shared_ptr<sigrok::Context> context, QWidget *parent = 0);
	~About();

private:
	Ui::About *ui;
	std::auto_ptr<QTextDocument> supportedDoc;
};

} // namespace dialogs
} // namespace pv

#endif // PULSEVIEW_PV_ABOUT_HPP
