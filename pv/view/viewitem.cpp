/*
 * This file is part of the PulseView project.
 *
 * Copyright (C) 2013 Joel Holdsworth <joel@airwebreathe.org.uk>
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

#include "viewitem.hpp"

#include <climits>

#include <QApplication>
#include <QMenu>
#include <QPalette>

namespace pv {
namespace view {

const int ViewItem::HighlightRadius = 6;

ViewItem::ViewItem() :
	context_parent_(NULL),
	selected_(false),
	drag_point_(INT_MIN, INT_MIN)
{
}

bool ViewItem::selected() const
{
	return selected_;
}

void ViewItem::select(bool select)
{
	selected_ = select;
}

bool ViewItem::dragging() const
{
	return drag_point_.x() != INT_MIN && drag_point_.y() != INT_MIN;
}

QPoint ViewItem::drag_point() const
{
	return drag_point_;
}

void ViewItem::drag()
{
	drag_point_ = point();
}

void ViewItem::drag_release()
{
	drag_point_ = QPoint(INT_MIN, INT_MIN);
}

QMenu* ViewItem::create_context_menu(QWidget *parent)
{
	context_parent_ = parent;
	return new QMenu(parent);
}

void ViewItem::delete_pressed()
{
}

QPen ViewItem::highlight_pen()
{
	return QPen(QApplication::palette().brush(
		QPalette::Highlight), HighlightRadius,
		Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
}

QColor ViewItem::select_text_colour(QColor background)
{
	return (background.lightness() > 64) ? Qt::black : Qt::white;
}

} // namespace view
} // namespace pv
