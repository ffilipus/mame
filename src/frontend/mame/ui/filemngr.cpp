// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    ui/filemngr.cpp

    MESS's clunky built-in file manager

    TODO
        - Restrict directory listing by file extension

*********************************************************************/

#include "emu.h"
#include "ui/ui.h"
#include "ui/menu.h"
#include "ui/filemngr.h"
#include "ui/filesel.h"
#include "ui/miscmenu.h"
#include "ui/imgcntrl.h"
#include "ui/floppycntrl.h"
#include "softlist.h"


/***************************************************************************
    FILE MANAGER
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

ui_menu_file_manager::ui_menu_file_manager(mame_ui_manager &mui, render_container *container, const char *warnings) : ui_menu(mui, container), selected_device(nullptr)
{
	// This warning string is used when accessing from the force_file_manager call, i.e.
	// when the file manager is loaded top front in the case of mandatory image devices
	if (warnings)
		m_warnings.assign(warnings);
	else
		m_warnings.clear();

	m_curr_selected = FALSE;
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

ui_menu_file_manager::~ui_menu_file_manager()
{
}


//-------------------------------------------------
//  custom_render - perform our special rendering
//-------------------------------------------------

void ui_menu_file_manager::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	const char *path;

	// access the path
	path = selected_device ? selected_device->filename() : nullptr;
	extra_text_render(top, bottom, origx1, origy1, origx2, origy2, nullptr, path);
}


void ui_menu_file_manager::fill_image_line(device_image_interface *img, std::string &instance, std::string &filename)
{
	// get the image type/id
	instance = string_format("%s (%s)", img->instance_name(), img->brief_instance_name());

	// get the base name
	if (img->basename() != nullptr)
	{
		filename.assign(img->basename());

		// if the image has been loaded through softlist, also show the loaded part
		if (img->part_entry() != nullptr)
		{
			const software_part *tmp = img->part_entry();
			if (tmp->name() != nullptr)
			{
				filename.append(" (");
				filename.append(tmp->name());
				// also check if this part has a specific part_id (e.g. "Map Disc", "Bonus Disc", etc.), and in case display it
				if (img->get_feature("part_id") != nullptr)
				{
					filename.append(": ");
					filename.append(img->get_feature("part_id"));
				}
				filename.append(")");
			}
		}
	}
	else
		filename.assign("---");
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_file_manager::populate()
{
	std::string tmp_inst, tmp_name;
	bool first_entry = true;

	if (!m_warnings.empty())
	{
		item_append(m_warnings.c_str(), nullptr, MENU_FLAG_DISABLE, nullptr);
		item_append("", nullptr, MENU_FLAG_DISABLE, nullptr);
	}

	// cycle through all devices for this system
	std::unordered_set<std::string> devtags;
	for (device_t &dev : device_iterator(machine().root_device()))
	{
		bool tag_appended = false;
		if (!devtags.insert(dev.tag()).second)
			continue;

		// check whether it owns an image interface
		image_interface_iterator subiter(dev);
		if (subiter.first() != nullptr)
		{
			// if so, cycle through all its image interfaces
			for (device_image_interface &scan : subiter)
			{
				if (!scan.user_loadable())
					continue;

				// if it is a children device, and not something further down the device tree, we want it in the menu!
				if (strcmp(scan.device().owner()->tag(), dev.tag()) == 0)
					if (devtags.insert(scan.device().tag()).second)
					{
						// check whether we already had some devices with the same owner: if not, output the owner tag!
						if (!tag_appended)
						{
							if (first_entry)
								first_entry = false;
							else
								item_append(ui_menu_item_type::SEPARATOR);
							item_append(string_format("[root%s]", dev.tag()).c_str(), nullptr, 0, nullptr);
							tag_appended = true;
						}
						// finally, append the image interface to the menu
						fill_image_line(&scan, tmp_inst, tmp_name);
						item_append(tmp_inst.c_str(), tmp_name.c_str(), 0, (void *)&scan);
					}
			}
		}
	}
	item_append(ui_menu_item_type::SEPARATOR);
	item_append("Reset",  nullptr, 0, (void *)1);

	custombottom = ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
}


//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_file_manager::handle()
{
	// process the menu
	const ui_menu_event *event = process(0);
	if (event != nullptr && event->itemref != nullptr && event->iptkey == IPT_UI_SELECT)
	{
		if ((FPTR)event->itemref == 1)
		{
			if (m_curr_selected)
				machine().schedule_hard_reset();
		}
		else
		{
			selected_device = (device_image_interface *) event->itemref;
			if (selected_device != nullptr)
			{
				m_curr_selected = TRUE;
				floppy_image_device *floppy_device = dynamic_cast<floppy_image_device *>(selected_device);
				if (floppy_device != nullptr)
				{
					ui_menu::stack_push(global_alloc_clear<ui_menu_control_floppy_image>(ui(), container, floppy_device));
				} 
				else
				{
					ui_menu::stack_push(global_alloc_clear<ui_menu_control_device_image>(ui(), container, selected_device));
				}
				// reset the existing menu
				reset(UI_MENU_RESET_REMEMBER_POSITION);
			}
		}
	}
}

// force file manager menu
void ui_menu_file_manager::force_file_manager(mame_ui_manager &mui, render_container *container, const char *warnings)
{
	// reset the menu stack
	ui_menu::stack_reset(mui.machine());

	// add the quit entry followed by the game select entry
	ui_menu *quit = global_alloc_clear<ui_menu_quit_game>(mui, container);
	quit->set_special_main_menu(true);
	ui_menu::stack_push(quit);
	ui_menu::stack_push(global_alloc_clear<ui_menu_file_manager>(mui, container, warnings));

	// force the menus on
	mui.show_menu();

	// make sure MAME is paused
	mui.machine().pause();
}
